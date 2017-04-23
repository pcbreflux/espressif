/* This is the CA certificate for the CA trust chain of
   Key in PEM format, as dumped via:

	using self signed server.crt as input

   this dont work for me:

   	openssl s_client -showcerts -connect localhost:8883 </dev/null

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
"MIIFSjCCBDKgAwIBAgIJAIBcumG2WDTYMA0GCSqGSIb3DQEBDQUAMGoxFzAVBgNV\r\n"
"BAMMDkFuIE1RVFQgYnJva2VyMRYwFAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYD\r\n"
"VQQLDAtnZW5lcmF0ZS1DQTEhMB8GCSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUu\r\n"
"bmV0MB4XDTE3MDQyMTE2MzE0M1oXDTMyMDQxNzE2MzE0M1owZzEUMBIGA1UEAwwL\r\n"
"cmFzcGJlcnJ5cGkxFjAUBgNVBAoMDU93blRyYWNrcy5vcmcxFDASBgNVBAsMC2dl\r\n"
"bmVyYXRlLUNBMSEwHwYJKoZIhvcNAQkBFhJub2JvZHlAZXhhbXBsZS5uZXQwggEi\r\n"
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDesJqQpUn5GQf3xQnttOulgyc2\r\n"
"XoiaVeYlGmKjy815DCHn6xvR1Iic1/g1A3JqVeURiMCEjQJrmiTKmzhDLCQsghJz\r\n"
"HdvwxxhrL97YwnnU4wu4okaEKlg7miHPXxp/FYY6slOh76pmgo1RU7K24hf+BeBJ\r\n"
"s7N3aciiZAXdLwga5kH2q8jGG5SNaTuHleE6BTeCPtXFM6cQLNs7v7cfTE49j+c/\r\n"
"39VvWbC5e1CXWUFZnm95S4mtR3XDKgYUGUVcK0Sx+CNUsuUqM8yafUrO1xZLt867\r\n"
"qE1kr+OpO+phCdDxRT/pi2TGeoocOdCmTbNRfe9AQz0xhTa2v5pqut0F9YZ7AgMB\r\n"
"AAGjggH0MIIB8DAMBgNVHRMBAf8EAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDALBgNV\r\n"
"HQ8EBAMCBeAwIQYJYIZIAYb4QgENBBQWEkJyb2tlciBDZXJ0aWZpY2F0ZTAdBgNV\r\n"
"HQ4EFgQUiAkaoMc6xXFgDNMJl9KAqu+ONc4wgZwGA1UdIwSBlDCBkYAUCehBeA2B\r\n"
"ttfgwce9sYVtPq0hw5ehbqRsMGoxFzAVBgNVBAMMDkFuIE1RVFQgYnJva2VyMRYw\r\n"
"FAYDVQQKDA1Pd25UcmFja3Mub3JnMRQwEgYDVQQLDAtnZW5lcmF0ZS1DQTEhMB8G\r\n"
"CSqGSIb3DQEJARYSbm9ib2R5QGV4YW1wbGUubmV0ggkA4jBO+9ddJ5wwVgYDVR0R\r\n"
"BE8wTYcEwKgCdocQ/oAAAAAAAAD8a5I6pz8yUYcQ/oAAAAAAAABJE3GvaOn44ocE\r\n"
"fwAAAYcQAAAAAAAAAAAAAAAAAAAAAYIJbG9jYWxob3N0MIGGBgNVHSAEfzB9MHsG\r\n"
"AysFCDB0MBwGCCsGAQUFBwIBFhBodHRwOi8vbG9jYWxob3N0MFQGCCsGAQUFBwIC\r\n"
"MEgwEBYJT3duVHJhY2tzMAMCAQEaNFRoaXMgQ0EgaXMgZm9yIGEgbG9jYWwgTVFU\r\n"
"VCBicm9rZXIgaW5zdGFsbGF0aW9uIG9ubHkwDQYJKoZIhvcNAQENBQADggEBAH4Y\r\n"
"7Pav2qbkgTuguoO6aKmgfMdy5QaJW9CbLR1XCfXoolmZt0PMjHL3T5ZCupuNn26t\r\n"
"NQ+LgbjYN8Jo1j9vy6u2gwp9XFifw+nbp4/SlOExwl+3cqMx80Ej9e6ZH8T3knal\r\n"
"LScgkDX5YzwTVh5gLAGkGyBNkMi1TicFB+QAFMRfDFwfZqxitymWRhSwZM6j3aD8\r\n"
"x7IbjcSmxilq/aJYsrfgEQWWlQ5i+OBHQBGPfEV93OiyO9REwAwxJl3hLF+eyR0B\r\n"
"2T0sRHE/r+OQzCv0ueoqicKiyp4HGtYo1V6K+9zE2zodbzpTrjzs3HD0onpTh7++\r\n"
"JzCptmdoJZ8o1wb1Km4=\r\n"
"-----END CERTIFICATE-----\r\n";


