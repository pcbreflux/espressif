#generate Keyfile:
#-----------------
#
#openssl req -nodes -newkey rsa:4096 -keyout prvtkey.pem -out cacert.pem -days 365 -subj '/CN=localhost'

openssl genrsa -out prvtkey.pem 2048
openssl genrsa -out cakey.pem 2048
openssl req -out ca_x509.req -key cakey.pem -new -config ./cert.conf
openssl req -out certx509.req -key prvtkey.pem -new -config ./cert.conf
openssl x509 -req -in ca_x509.req -out ca_x509.pem -sha1 -days 5000 -signkey cakey.pem
openssl x509 -req -in certx509.req -out cacert.pem -sha1 -CAcreateserial -days 5000 -CA ca_x509.pem -CAkey cakey.pem
rm ca_x509.req certx509.req ca_x509.pem certx509.pem ca_x509.srl








