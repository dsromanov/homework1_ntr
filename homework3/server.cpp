#include <RRCConnectionRequest.h>
#include <RRCConnectionSetup.h>
#include <RRCConnectionSetupComplete.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdio>

#define PORT 1234
#define BUFFER_SIZE 1024

bool validateRequest(RRCConnectionRequest_t* request) {
    if (request->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf == (uint8_t*)"" ||
            request->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.size <= 0)
        return true;
    return false;
}
RRCConnectionSetup_t* createSetupResponse(bool request_status) {
    RRCConnectionSetup_t* setup = (RRCConnectionSetup_t*)calloc(1, sizeof(RRCConnectionSetup_t));
    if (!setup) {
        perror("calloc fail");
        exit(1);
    }

    setup->rrcConnectionSetup_r8.lateNonCriticalExtension = (OCTET_STRING*)calloc(1, 16);
    setup->rrcConnectionSetup_r8.lateNonCriticalExtension->size = 10;

    if (!request_status) {
        setup->rrc_TransactionIdentifier = 0;
        setup->rrcConnectionSetup_r8.lateNonCriticalExtension->buf = (uint8_t*)"good_request";
    } else {
        setup->rrc_TransactionIdentifier = 1;
        setup->rrcConnectionSetup_r8.lateNonCriticalExtension->buf = (uint8_t*)"bad_request";
    }

    return setup;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("failed creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt fail");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        perror("bind fail");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    int addr_len = sizeof(struct sockaddr_in);
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    char request_buffer[BUFFER_SIZE];
    int request_read = read(client_fd, request_buffer, BUFFER_SIZE);
    if (request_read < 0) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }

    RRCConnectionRequest_t* received_request = 0;
    asn_dec_rval_t request_rvalue = ber_decode(0, &asn_DEF_RRCConnectionRequest, (void**)&received_request, request_buffer, sizeof(request_buffer));
    if (request_rvalue.code!= RC_OK) {
        printf("failed to decode message!");
        ASN_STRUCT_FREE(asn_DEF_RRCConnectionRequest, received_request);
        return 0;
    }

    printf("RRC Connection Request(client):\n");
    xer_fprint(stdout, &asn_DEF_RRCConnectionRequest, received_request);

    bool request_status = validateRequest(received_request);

    RRCConnectionSetup_t* setup = createSetupResponse(request_status);

    asn_enc_rval_t setup_enc;
    uint8_t setup_buffer[sizeof(setup) * 5];
    setup_enc = der_encode_to_buffer(&asn_DEF_RRCConnectionSetup, setup, setup_buffer, sizeof(setup_buffer));
    if (setup_enc.encoded == -1) {
        fprintf(stderr, "Failed to encode RRCConnectionRequest");
        exit(1);
    }

    send(client_fd, setup_buffer, BUFFER_SIZE, 0);

    char setup_complete_buffer[BUFFER_SIZE];
    int setup_complete_read = read(client_fd, setup_complete_buffer, BUFFER_SIZE);
    if (setup_complete_read < 0) {
        perror("read fail");
        exit(EXIT_FAILURE);
    }

    RRCConnectionSetupComplete_t* received_setup_complete = 0;
    asn_dec_rval_t setup_complete_rvalue = ber_decode(0, &asn_DEF_RRCConnectionSetupComplete, (void**)&received_setup_complete, received_setup_completelt_buffer, sizeof(setup_complete_buffer));
    if (setup_complete_rvalue.code!= RC_OK) {
        printf("failed to decode message!");
        ASN_STRUCT_FREE(asn_DEF_RRCConnectionSetupComplete, received_setup_complete);
        return 0;
    }

    printf("RRCConnectionSetupComplete:\n");
    xer_fprint(stdout, &asn_DEF_RRCConnectionSetupComplete, received_setup_complete);

    close(client_fd);
    return 0;
}



