/* This is the CA certificate for the CA trust chain of
   www.howsmyssl.com in PEM format, as dumped via:

   openssl s_client -showcerts -connect abc.cloudmqtt.com:12345 </dev/null

   The CA cert is the last cert in the chain output by the server.
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 1 s:/C=US/O=Let's Encrypt/CN=Let's Encrypt Authority X3
   i:/O=Digital Signature Trust Co./CN=DST Root CA X3
 */
const char *server_root_cert = "-----BEGIN CERTIFICATE-----\r\n"
"MIIFMDCCBBigAwIBAgIQV5SFWjNqKQ7ivjk88Axt3zANBgkqhkiG9w0BAQsFADBv\r\n"
"MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\r\n"
"ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\r\n"
"eHRlcm5hbCBDQSBSb290MB4XDTE0MTIyMjAwMDAwMFoXDTIwMDUzMDEwNDgzOFow\r\n"
"gZQxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\r\n"
"BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMTowOAYD\r\n"
"VQQDEzFDT01PRE8gU0hBLTI1NiBEb21haW4gVmFsaWRhdGlvbiBTZWN1cmUgU2Vy\r\n"
"dmVyIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApe7aGrg+6WaE\r\n"
"MyvsWocOnfpDXsxsmrOezwlTbnGbsFkuEU4wuOIVLaL2F4H5gX9fFinG7AFz7tdG\r\n"
"6wq0if2JvKj4us9ewrm+NPvu/pAKAXcr4TavNHLBXeUWdpLCjP/gaAev9W3O07l4\r\n"
"X8jxZ0DBh1hPUnVyGKUveZCwBAGPasSYlQfo4PcLPilqW/vHZCCOV8l8HW0CeuPO\r\n"
"VPAIZYaXicngMuyRGo790YSj759Gu8wAMHxMBelVSbBwgb1gAYqANFuLJQvC57oW\r\n"
"XYa8w/zKn/QJWkkfwkLyqNwhSbnQuoQByVSsGTf2ItQQzzQS+5nl1bF2pTZQfK7Q\r\n"
"W36GGtK2AwIDAQABo4IBoDCCAZwwHwYDVR0jBBgwFoAUrb2YejS0Jvf6xCZU7wO9\r\n"
"4CTLVBowHQYDVR0OBBYEFB6sP/wP4Sf1OpwFluiLM3n3dXnxMA4GA1UdDwEB/wQE\r\n"
"AwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggr\r\n"
"BgEFBQcDAjAbBgNVHSAEFDASMAYGBFUdIAAwCAYGZ4EMAQIBMEQGA1UdHwQ9MDsw\r\n"
"OaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVybmFs\r\n"
"Q0FSb290LmNybDCBswYIKwYBBQUHAQEEgaYwgaMwPwYIKwYBBQUHMAKGM2h0dHA6\r\n"
"Ly9jcnQudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVybmFsQ0FSb290LnA3YzA5\r\n"
"BggrBgEFBQcwAoYtaHR0cDovL2NydC51c2VydHJ1c3QuY29tL0FkZFRydXN0VVRO\r\n"
"U0dDQ0EuY3J0MCUGCCsGAQUFBzABhhlodHRwOi8vb2NzcC51c2VydHJ1c3QuY29t\r\n"
"MA0GCSqGSIb3DQEBCwUAA4IBAQCsPFl52og0fw7ZBzEwE0vqi8k2JA/nlQCkXv3A\r\n"
"rJ8aSqmXjuAwmXUXfZlnaZMWz8nVcT+DnmxWGeqCSqtB3Fvgtmg7fO22aGAJ3H/I\r\n"
"zTxPd0x+1BebdDpOB3XS/It5Wt8FGKtnJtM+jG76ESoy5DpwTHdwi+ZF9bDlJkBJ\r\n"
"NwImRkjJWMMPPIJsWE03d2qs6KjxBD/B11wf+aevKFIQ0dN6pt7Gc/0AepKDRW83\r\n"
"+7eYEqUoBfjcoeK86NSDwpHSXIqaVa3p0SFlSQ16VYE0YWHCPvCzI4bZJGlePq4g\r\n"
"z5Z2KDJ4sJ7bbAtAU44vI1wTv2aFpY45RAIS4nAFjvwYPRVY\r\n"
"-----END CERTIFICATE-----\r\n";


