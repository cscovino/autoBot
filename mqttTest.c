#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <mosquitto.h>

#define mqtt_host	"localhost"
#define mqtt_port	1883

static int keepRunning = 1;

void handle_signal(int s)
{
	keepRunning = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("\n[INFO]\tCONNECT CALLBACK, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	bool match = 0;
	
	printf("\ngot message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

	mosquitto_topic_matches_sub("msg/box", message->topic, &match);
	if (match) {
		printf("\ngot message for msg/box topic\n");
	}
}

int main(int argc, char *argv[])
{
	bool reconnect = true;
	char clientid[24];
	struct mosquitto *mosq;
	int rc = 0;

	signal(SIGINT, handle_signal);

	mosquitto_lib_init();

	memset(clientid, 0, 24);
	snprintf(clientid, 23, "omega2_%d", getpid());
	mosq = mosquitto_new(clientid, true, 0);

	if(mosq){
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_message_callback_set(mosq, message_callback);

	    rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

		mosquitto_subscribe(mosq, NULL, "msg/box", 0);

		while(keepRunning){
			rc = mosquitto_loop(mosq, -1, 1);
			if(keepRunning && rc){
				printf("connection error!\n");
				sleep(10);
				mosquitto_reconnect(mosq);
			}
		}
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();

	return rc;
}
