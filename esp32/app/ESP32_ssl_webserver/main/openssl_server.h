/* OpenSSL server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef _OPENSSL_DEMO_H_
#define _OPENSSL_DEMO_H_

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID               CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS               CONFIG_WIFI_PASSWORD

#define OPENSSL_DEMO_THREAD_NAME        "OpenSSL_demo"
#define OPENSSL_DEMO_THREAD_STACK_WORDS 10240
#define OPENSSL_DEMO_THREAD_PRORIOTY    8

#define OPENSSL_DEMO_RECV_BUF_LEN       4096
#define OPENSSL_DEMO_SEND_BUF_LEN       4096

#define OPENSSL_DEMO_LOCAL_TCP_PORT     443

#define OPENSSL_DEMO_LOCAL_TCP_MTU     1440

const char OPENSSL_DEMO_HTTP_200_TXT[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char OPENSSL_DEMO_HTTP_200_PNG[] = "HTTP/1.1 200 OK\r\nPragma: public\r\nCache-Control: max-age=86400\r\nExpires: Thu, 26 Dec 2017 23:59:59 GMT\r\nContent-Type: image/png\r\n\r\n";
const char OPENSSL_DEMO_HTTP_HEAD[]  = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char OPENSSL_DEMO_HTTP_STYLE[] = "<style>body,textarea,input,select { background: 0; border-radius: 0; font: 16px sans-serif; margin: 0}textarea,input,select { outline: 0; font-size: 14px; border: 1px solid #ccc; padding: 8px; width: 90%}.btn,.btn_off a { text-decoration: none}.container { margin: auto; width: 90%}@media(min-width:1200px) { .container { margin: auto; width: 70%; }}@media(min-width:768px) and (max-width:1200px) { .container { margin: auto; width: 70%; }btn,btn_off}.btn,h2 { font-size: 2em}.btn_off { font-size: 2em}h1 { font-size: 3em}.btn { background: #112299; border-radius: 4px; border: 0; color: #fff; cursor: pointer; display: inline-block; margin: 2px 0; padding: 10px 14px 11px; width: 100%}.btn_off { background: #992211; border-radius: 4px; border: 0; color: #fff; cursor: pointer; display: inline-block; margin: 2px 0; padding: 10px 14px 11px; width: 100%}.btn:hover { background: #09d}.btn_off:hover { background: #90d}.btn:active,.btn:focus { background: #08b}.btn_off:active,.btn_off:focus { background: #80b}label>* { display: inline}form>* { display: block; margin-bottom: 10px}textarea:focus,input:focus,select:focus { border-color: #5ab}.msg { background: #def; border-left: 5px solid #59d; padding: 1.5em}.q { float: right; width: 64px; text-align: right}input[type='checkbox'] { float: left; width: 20px}.table td { padding:.5em; text-align:left}.table tbody>:nth-child(2n-1) { background:#ddd}</style>";
const char OPENSSL_DEMO_HTTP_SCRIPT[] = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char OPENSSL_DEMO_HTTP_HEAD_END[] = "</head><body><div class=\"container\">";
const char OPENSSL_DEMO_HTTP_LOGO[] = "<a id=\"logo\" href=\"/\"><img src=\"logo.png\" alt=\"logo\" width=\"147\" height=\"141\" border=\"0\"></a>";
const char OPENSSL_DEMO_HTTP_FORM_BUTTON_ON[] = "<form action=\"/{f}\" method=\"get\"><button class=\"btn\">{l}</button></form><br/>";
const char OPENSSL_DEMO_HTTP_FORM_BUTTON_OFF[] = "<form action=\"/{f}\" method=\"get\"><button class=\"btn_off\">{l}</button></form><br/>";
const char OPENSSL_DEMO_HTTP_END[] = "</div></body></html>";


#endif

