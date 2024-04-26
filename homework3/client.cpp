#include <RRCConnectionRequest.h>
#include <RRCConnectionSetup.h>
#include <RRCConnectionSetupComplete.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>

#define PORT 1234
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1024

RRCConnectionRequest_t* createConnectionRequest() {
    RRCConnectionRequest_t* request = (RRCConnectionRequest_t*)calloc(1, sizeof(RRCConnectionRequest_t));
    if (!request) {
        perror("calloc error");
        return nullptr;
    }
    request->criticalExtensions.present = criticalExtensions_PR_rrcConnectionRequest_r8;
    request->criticalExtensions.choice.rrcConnectionRequest_r8.spare.buf = (uint8_t*)"spare";
    request->criticalExtensions.choice.rrcConnectionRequest_r8.spare.size = 1;
    request->criticalExtensions.choice.rrcConnectionRequest_r8.establishmentCause = 1;
    request->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.present = InitialUE_Identity_PR_randomValue;
    request->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf = (uint8_t*)"random_value";
    request->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.size = 16;
    return request;
}

int main() {
    RRCConnectionRequest_t* request = createConnectionRequest();
    if (!request) {
        return 1;
    }

    asn_enc_rval_t enc;
    uint8_t request_buffer[sizeof(request) * 5];
    enc = der_encode_to_buffer(&asn_DEF_RRCConnectionRequest, request, request_buffer, sizeof(request_buffer));
    if (enc.encoded == -1) {
        fprintf(stderr, "Failed to encode RRCConnectionRequest\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error creating socket\n");
        return 1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP_ADDRESS, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return 1;
    }
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Failed to connect\n");
        return 1;
    }

    send(sock, request_buffer, sizeof(request_buffer), 0);

    uint8_t setup_buffer[BUFFER_SIZE];
    RRCConnectionSetup_t* setup = 0;
    asn_dec_rval_t setup_rvalue;
    read(sock, setup_buffer, sizeof(setup_buffer));

    setup_rvalue = ber_decode(0, &asn_DEF_RRCConnectionSetup, (void**)&setup, setup_buffer, sizeof(setup_buffer));
    if (setup_rvalue.code!= RC_OK) {
        printf("Error decoding the message");
        ASN_STRUCT_FREE(asn_DEF_RRCConnectionSetup, setup);
        return 1;
    }

    printf("\nRRC Connection Setup established, server output:\n");
    xer_fprint(stdout, &asn_DEF_RRCConnectionSetup, setup);

    RRCConnectionSetupComplete_t* setup_complete = (RRCConnectionSetupComplete_t*)calloc(1, sizeof(RRCConnectionSetupComplete_t));
    if (!setup_complete) {
        perror("calloc error");
        return 1;
    }

    setup_complete->rrc_TransactionIdentifier = (setup->rrc_TransactionIdentifier == 0) ? 0 : 1;
    setup_complete->c1.present = c1_PR_rrcConnectionSetupComplete_r8;
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.selectedPLMN_Identity = 1;
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.dedicatedInfoNAS.buf = (uint8_t*)"test info";
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.dedicatedInfoNAS.size = 8;
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.registeredMME = (RegisteredMME*)calloc(1, 32);
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.buf = (uint8_t*)"sgdgsd";
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.size = 16;
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.buf = (uint8_t*)"sdgsdgsd";
    setup_complete->c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.size = 8;

    asn_enc_rval_t setup_complete_enc;
    uint8_t setup_complete_buffer[sizeof(setup_complete) * 10];
    setup_complete_enc = der_encode_to_buffer(&asn_DEF_RRCConnectionSetupComplete, setup_complete, setup_complete_buffer, sizeof(setup_complete_buffer));
    if (setup_complete_enc.encoded == -1) {
        fprintf(stderr, "Failed to encode RRCConnectionRequest\n");
        return 1;
    }

    send(sock, setup_complete_buffer, sizeof(setup_complete_buffer), 0);

    close(sock);
    return 0;
}