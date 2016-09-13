#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <arguments.h>

args_t arguments = {
	.ip_address = NULL,
	.port = 7,
	.protocol = PROTOCOL_ICMP,
	.payload_size = 32,
	.timeoutMS = 4000,
	.count = 4,
	.interval = 1
};

bool isValidNamedArg(char* arg){
	return strlen(arg) > 1;
}

void parseArguments(int argc, char* argv[]){
	int error = 0;

	for(int i = 1; i < argc; i++){

		if(argv[i][0] == '-'){
	
			if(isValidNamedArg(argv[i])){
			
				switch(argv[i][1]){
					case 'm':
					switch(argv[i][3]){
						case PROTOCOL_TCP:
						arguments.protocol = PROTOCOL_TCP;
						break;
				
						case PROTOCOL_UDP:
						arguments.protocol = PROTOCOL_UDP;
						break;
					}
					break;

					case 'p':
					arguments.port = atol(&argv[i][3]);
					break;

					case 's':
					arguments.payload_size = atol(&argv[i][3]);
					break;

					case 't':
					arguments.timeoutMS = atol(&argv[i][3]);
					break;

					case 'c':
					arguments.count = atol(&argv[i][3]);
					break;

					case 'i':
					arguments.interval = atol(&argv[i][3]);
					break;;

					case 'h':
					print_help();
					exit(EXIT_SUCCESS);
					break;

					default:
					printf("argument::unknown %s\n", argv[i]);
					error = 1;
				}

			} else {
				printf("arguments::unknown %s\n", argv[i]);
				error = 1;
			}

		} else {
			arguments.ip_address = argv[i];
		}

	}

	if(arguments.ip_address == NULL){
		printf("arguments::mandatory No IP address provided\n");
		print_help();
		exit(EXIT_FAILURE);
	}

	if(error){
		print_help();
		exit(EXIT_FAILURE);
	}
}
