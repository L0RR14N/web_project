#include "./mongoose/mongoose.h"
#include "./input/input.h"
#include "./constants/constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Коды ошибок
enum {
    ERR_OK                  = 0,
    ERR_MISSING_ENV         = 1,
    ERR_INVALID_CREDENTIALS = 2,
    ERR_FILE_NOT_FOUND      = 3
};

// Функция для получения комментария по среднему баллу
static const char *get_comment(double average) {
    if (average >= 4.5) return "Отлично";
    if (average >= 3.5) return "Хорошо";
    if (average >= 2.5) return "Удовлетворительно";
    return "Неудовлетворительно";
}

// Основной обработчик запросов
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        int status_code = 500;
        const char *ctype = "";
        char *response = NULL;
        int error_code = ERR_FILE_NOT_FOUND;

        // Обработка POST запроса на /login
        if (!mg_strcmp(hm->uri, mg_str("/login")) &&
            !mg_strcasecmp(hm->method, mg_str("POST"))) {

            char username[100], password[100];
            const char *expected_user = getenv("LOGIN_USER");
            const char *expected_pass = getenv("LOGIN_PASS");

            if (!expected_user || !expected_pass) {
                error_code = ERR_MISSING_ENV;
				mg_http_reply(c, 500, "", "Server configuration error");
				return;

            } else {
                mg_http_get_var(&hm->body, "username", username, sizeof(username));
                mg_http_get_var(&hm->body, "password", password, sizeof(password));

                if (!strcmp(username, expected_user) &&
                    !strcmp(password, expected_pass)) {
                    response = read_file(PATH_GRADES_HTML);
                    error_code = ERR_OK;
                } else {
                    response = read_file(PATH_ERROR_HTML);
                    error_code = ERR_INVALID_CREDENTIALS;
                }

                if (response) {
                    status_code = 200;
                    ctype = CONTENT_TYPE_HTML;
                }
            }
        }
        // Обработка запроса стилей
        else if (!mg_strcmp(hm->uri, mg_str("/styles.css"))) {
            response = read_file(PATH_CSS_STYLES);
            if (response) {
                status_code = 200;
                ctype = CONTENT_TYPE_CSS;
                error_code = ERR_OK;
            }
        }
        // Обработка POST запроса на /calculate (расчет среднего балла)
        else if (!mg_strcmp(hm->uri, mg_str("/calculate")) && 
                 !mg_strcasecmp(hm->method, mg_str("POST"))) {
            char s1[10], s2[10], s3[10], s4[10], s5[10];
            mg_http_get_var(&hm->body, "subject1", s1, sizeof(s1));
            mg_http_get_var(&hm->body, "subject2", s2, sizeof(s2));
            mg_http_get_var(&hm->body, "subject3", s3, sizeof(s3));
            mg_http_get_var(&hm->body, "subject4", s4, sizeof(s4));
            mg_http_get_var(&hm->body, "subject5", s5, sizeof(s5));

            double sum = atof(s1) + atof(s2) + atof(s3) + atof(s4) + atof(s5);
            double average = sum / 5.0;

            char avg_str[10];
            snprintf(avg_str, sizeof(avg_str), "%.2f", average);

            response = read_file(PATH_RESULT_HTML);
            if (response) {
                char *temp = replace_placeholders(response, "{{average}}", avg_str);
                free(response);
                response = replace_placeholders(temp, "{{comment}}", get_comment(average));
                if (temp) free(temp);
                status_code = 200;
                ctype = CONTENT_TYPE_HTML;
                error_code = ERR_OK;
            }
        }
        // Обработка запроса страницы с оценками
        else if (!mg_strcmp(hm->uri, mg_str("/grades"))) {
            response = read_file(PATH_GRADES_HTML);
            if (response) {
                status_code = 200;
                ctype = CONTENT_TYPE_HTML;
                error_code = ERR_OK;
            }
        }
        // Все остальные запросы - страница входа
        else {
            response = read_file(PATH_LOGIN_HTML);
            if (response) {
                status_code = 200;
                ctype = CONTENT_TYPE_HTML;
                error_code = ERR_OK;
            }
        }

        // Отправка ответа
        if (error_code == ERR_OK) {
            mg_http_reply(c, status_code, ctype, "%s", response);
        } else {
            mg_http_reply(c, 500, "", "");
        }

        if (response) free(response);
    }
}

int main(void) {
	printf("LOGIN_USER=%s\n", getenv("LOGIN_USER"));
	printf("LOGIN_USER=%s\n", getenv("LOGIN_PASS"));
	if (!getenv("LOGIN_USER") || !getenv("LOGIN_PASS")) {
		fprintf(stderr, "[ERROR] LOGIN_USER and LOGIN_PASS must be set\n");
		return 1;
	}

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    const char *addr = "http://localhost:8080";
    if (!mg_http_listen(&mgr, addr, fn, NULL)) {
        fprintf(stderr, "Failed to start server on %s\n", addr);
        return 1;
    }

    printf("Server running on %s\n", addr);
    while (true) mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return 0;
}
