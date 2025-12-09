# Configuration

Copy the template file `secrets.cmake.in` to your own `secrets.cmake` and replace the placeholders with your real keys.

## Card Keys

Those are 16 byte hexadecimal numbers you can just generate randomly. You need to take care not to loose the master key - otherwise you can throw the cards programmed with that key away, they will stay inaccessible forever.

* `CARD_PICC_MASTER_AES_KEY`  
For global access to the card "root"

* `CARD_DOORLOCK_MASTER_AES_KEY`  
For access to the "doorlock" application section

* `CARD_DOORLOCK_WRITE_AES_KEY`  
For writing to the user ID file of this application

* `CARD_DOORLOCK_READ_AES_KEY`  
For reading from the user ID file of this application

## MQTT Keys

### Secure MQTT Messaging

* `MQTT_CA_CERT`  
public CA key used to sign the MQTT brokers public key (from `ca.crt`)

### MQTT Client Authentication

* `MQTT_CLIENT_CERT`  
clients public, CA-certified key (from `client.crt`)

* `MQTT_CLIENT_KEY`  
clients private key (from `client.key`)