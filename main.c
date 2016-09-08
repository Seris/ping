#include <stdio.h>
#include <stdlib.h>

#include <protocols.h>
#include <arguments.h>

extern args_t arguments;

int main(int argc, char* argv[]){
	parseArguments(argc, argv);

	protocol_stack_t *stack = get_protocol_stack();

	if(stack != NULL){
		execute_protocol(stack);
	} else {
		printf("protocol::notsupported\nProtocol yet supported\n");
	}

	return EXIT_SUCCESS;
}

