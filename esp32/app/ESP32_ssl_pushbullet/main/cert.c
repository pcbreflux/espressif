/* This is the CA certificate for the CA trust chain of
   smtp.gmail.com in PEM format, as dumped via:

   openssl s_client -showcerts -servername api.pushbullet.com -connect api.pushbullet.com:443 </dev/null

   The CA cert is the last cert in the chain output by the server.
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 1 s:/C=US/O=GeoTrust Inc./CN=RapidSSL SHA256 CA - G2
   i:/C=US/O=GeoTrust Inc./OU=(c) 2008 GeoTrust Inc. - For authorized use only/CN=GeoTrust Primary Certification Authority - G3
 */
const char *server_root_cert =      "-----BEGIN CERTIFICATE-----\n" \
"MIIEtTCCA52gAwIBAgIQSOmUQNQ2SRy4uII9CUOUxzANBgkqhkiG9w0BAQsFADCB\n" \
"mDELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUdlb1RydXN0IEluYy4xOTA3BgNVBAsT\n" \
"MChjKSAyMDA4IEdlb1RydXN0IEluYy4gLSBGb3IgYXV0aG9yaXplZCB1c2Ugb25s\n" \
"eTE2MDQGA1UEAxMtR2VvVHJ1c3QgUHJpbWFyeSBDZXJ0aWZpY2F0aW9uIEF1dGhv\n" \
"cml0eSAtIEczMB4XDTE0MDYxMDAwMDAwMFoXDTI0MDYwOTIzNTk1OVowRzELMAkG\n" \
"A1UEBhMCVVMxFjAUBgNVBAoTDUdlb1RydXN0IEluYy4xIDAeBgNVBAMTF1JhcGlk\n" \
"U1NMIFNIQTI1NiBDQSAtIEcyMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n" \
"AQEAxJVjKNBOMEWvi5c0FEX4XFhK+jOObpxgq/OG/zR0siu+oYzVoqNgekC54fwi\n" \
"yme6YKrHmvkGf+73uoUFsAP/cq4VQUqYZNcXS1TvBcaYB5MnPk/cD8Z7i+fzBl6N\n" \
"6LSuKbQeHi0WkNPqqueMO22vNln/xQr6x0y9NotkxEr1zjP5B75/RZCoCBSw0KVP\n" \
"34KA2hvuwxOwmPUP+X52tea5XWi5XFCQiaQ2sXAW6rEQtWp23+G7/HjycpnPyaLU\n" \
"c1R3v8A5d+WuEsV4WhlF1EEZ03z1b5lr14u8LQmdSxBhwNpSw68iQ8brN35jdDAN\n" \
"anGO3l1bisjF15sp6K62JWGB6wIDAQABo4IBSTCCAUUwLgYIKwYBBQUHAQEEIjAg\n" \
"MB4GCCsGAQUFBzABhhJodHRwOi8vZy5zeW1jZC5jb20wEgYDVR0TAQH/BAgwBgEB\n" \
"/wIBADBMBgNVHSAERTBDMEEGCmCGSAGG+EUBBzYwMzAxBggrBgEFBQcCARYlaHR0\n" \
"cDovL3d3dy5nZW90cnVzdC5jb20vcmVzb3VyY2VzL2NwczA2BgNVHR8ELzAtMCug\n" \
"KaAnhiVodHRwOi8vZy5zeW1jYi5jb20vR2VvVHJ1c3RQQ0EtRzMuY3JsMA4GA1Ud\n" \
"DwEB/wQEAwIBBjApBgNVHREEIjAgpB4wHDEaMBgGA1UEAxMRU3ltYW50ZWNQS0kt\n" \
"MS02OTcwHQYDVR0OBBYEFEz0v+g7vsIk8xtHO7VuSI4Wq68SMB8GA1UdIwQYMBaA\n" \
"FMR5yo6hTgMdHNxr2zFblD4/MH8tMA0GCSqGSIb3DQEBCwUAA4IBAQB6U7Xetu9S\n" \
"o1+K9YnxQsxeRoiupQiHUd4PDwLrDIJ443N9cb1D6cqKP+AlkpszM3RJXgDZcxQc\n" \
"C0Z2HIoNTYxsfkv3YNiBeKB40CViqxDKIugcGd1Sg2QF5Ydmrud6pDs+2HB6dqJn\n" \
"OdTJ+uW3HkHiCTmIHBhVCsRBr7Lz8w9CFGF0gePah1qaTYvTyY+JZhMpEeT/4t+O\n" \
"lgxaoaprm/38AztVDaaiJUgXH0Ko2mx+aW6g32fSbfQOahJ59XzIpTIcxDGy5ruo\n" \
"a2qiimBpwFd9svIxDJhlMuwIWs7GmOkhlz8seSkD9faUK1Mx85NoV+HXTzrRYaFg\n" \
"zrmrmK41VGOL\n" \
"-----END CERTIFICATE-----\n";




