
char *document_root;
// из запроса вытащить метод -- первым словом
    // switch-case по методам
// урл / процентный урл -- вторым словом вытащить
// сформировать ответ
// отправить ответ

void init_document_root(char *new_document_root) {
    document_root = new_document_root;
}

int read_http_request(FILE * fin) {
    // считать в буфер до '\r\n',
    // если буфер переполнен -- завершить
}

// сметчить на тип "запрос" считанное из сокета
//int method_len = sscanf(buf, "%s %s %s\n", request->method,
//                        request->uri,
//                        request->version);
//if (method_len != 3) {
//return FAIL;
//}

int get_http_response_cb(int sock_fd) {
//считать запрос
//распарсить запрос
//свич-кейс по методам
//сформировать ответ
//вернуть ответ
}

