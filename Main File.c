#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define MAX_PACKET_SIZE 16
#define CRC_DIVISOR_LEN 20
#define REPORT_FILE_PATH "network_report.txt"

typedef struct {
    unsigned char data[MAX_PACKET_SIZE];
    unsigned int length;
} NetworkPacket;

// Function Prototypes
void printMenu();
int processUserInput();
void simulatePacketCapture(NetworkPacket *packet);
void performCRC(char *input, char *divisor, int dataLen, int divLen);
bool performCRCCheck(char *received, char *divisor, int dataLen, int divLen);
bool performParityCheck(const NetworkPacket *packet);
void generateReport(const NetworkPacket *packet, const char* reportFilePath);

NetworkPacket capturedPacket;
char divisor[CRC_DIVISOR_LEN] = "1101"; // Example divisor for CRC checking
bool isPacketCaptured = false;
int main() {
    srand((unsigned int)time(NULL));
    int userChoice = 0;

    while (userChoice != 5) {
        printMenu();
        userChoice = processUserInput();

        switch (userChoice) {
            case 1:
                simulatePacketCapture(&capturedPacket);
                isPacketCaptured = true;
                printf("Packet captured.\n");
                break;
            case 2:
                if (isPacketCaptured && performCRCCheck((char *)capturedPacket.data, divisor, capturedPacket.length, strlen(divisor))) {
                    printf("Packet passed CRC check.\n");
                } else {
                    printf("Packet failed CRC check or no packet captured yet.\n");
                }
                break;
            case 3:
                if (isPacketCaptured && performParityCheck(&capturedPacket)) {
                    printf("Packet passed parity check.\n");
                } else {
                    printf("Packet failed parity check or no packet captured yet.\n");
                }
                break;
            case 4:
                if (isPacketCaptured) {
                    generateReport(&capturedPacket, REPORT_FILE_PATH);
                } else {
                    printf("No packet captured yet to generate report.\n");
                }
                break;
        }
    }

    return 0;
}

void printMenu() {
    printf("\nMenu:\n");
    printf("1. Capture Packet\n");
    printf("2. Perform CRC Check\n");
    printf("3. Perform Parity Check\n");
    printf("4. Generate Report\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

int processUserInput() {
    int choice;
    scanf("%d", &choice);
    while (getchar() != '\n');  // Clear the input buffer
    return choice;
}

void simulatePacketCapture(NetworkPacket *packet) {
    // Simulate packet capturing
    packet->length = rand() % (MAX_PACKET_SIZE + 1); // Random length between 1 and MAX_PACKET_SIZE
    for (size_t i = 0; i < packet->length; i++) {
        packet->data[i] = rand() % 256; // Random byte
    }
    isPacketCaptured = true;
}

void printData(const unsigned char *data, size_t length) {
    printf("Decimal: ");
    for (size_t i = 0; i < length; i++) {
        printf("%d ", data[i]);
    }
    printf("\nBinary: ");
    for (size_t i = 0; i < length; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

bool performParityCheck(const NetworkPacket *packet) {
    int parity = 0;
    printf("Data before Parity Check:\n");
    printData(packet->data, packet->length);
    for (size_t i = 0; i < packet->length; i++) {
        for (size_t bit = 0; bit < 8; bit++) {
            parity ^= (packet->data[i] >> bit) & 1;
        }
    }
    printf("Parity Bit: %d\n", parity); // Parity will be 0 or 1
    return parity == 0; // Assuming even parity
}

void generateReport(const NetworkPacket *packet, const char* reportFilePath) {
    FILE *reportFile = fopen(reportFilePath, "a");
    if (reportFile != NULL) {
        fprintf(reportFile, "Packet Length: %u\n", packet->length);
        fprintf(reportFile, "Packet Data (Decimal): ");
        for (size_t i = 0; i < packet->length; i++) {
            fprintf(reportFile, "%d ", packet->data[i]);
        }
        fprintf(reportFile, "\nPacket Data (Binary): ");
        for (size_t i = 0; i < packet->length; i++) {
            for (int j = 7; j >= 0; j--) {
                fprintf(reportFile, "%d", (packet->data[i] >> j) & 1);
            }
            fprintf(reportFile, " ");
        }
        fprintf(reportFile, "\n\n");
        fclose(reportFile);
        printf("Report generated and saved to %s\n", reportFilePath);
    } else {
        printf("Unable to open report file %s\n", reportFilePath);
    }
}

void performCRC(char *input, char *divisor, int dataLen, int divLen) {
    // Create an array to hold the data plus the CRC value (initialized to zero)
    unsigned char crc[dataLen + divLen - 1];
    memset(crc, 0, sizeof(crc));

    // Copy the input data into the crc array
    for (int i = 0; i < dataLen; ++i) {
        crc[i] = input[i] == '1' ? 1 : 0; // Convert ASCII '0'/'1' to binary 0/1
    }

    // Perform the bitwise CRC division
    for (int i = 0; i <= dataLen; ++i) {
        if (crc[i] == 1) {
            for (int j = 0; j < divLen; ++j) {
                crc[i + j] ^= (divisor[j] == '1' ? 1 : 0); // Perform XOR with binary 0/1
            }
        }
    }

    // Append the CRC to the end of the input
    for (int i = 0; i < divLen - 1; ++i) {
        input[dataLen + i] = crc[dataLen + i] ? '1' : '0'; // Convert binary 0/1 back to ASCII '0'/'1'
    }
    input[dataLen + divLen - 1] = '\0'; // Null-terminate the result
}

bool performCRCCheck(char *received, char *divisor, int dataLen, int divLen) {
    // Copy the received data to a buffer including space for the CRC
    char receivedWithCRC[MAX_PACKET_SIZE + CRC_DIVISOR_LEN];
    strncpy(receivedWithCRC, received, dataLen + divLen - 1);

    printf("Data for CRC Check:\n");
    printData((unsigned char *)receivedWithCRC, dataLen + divLen - 1);

    // Perform CRC on the received data
    performCRC(receivedWithCRC, divisor, dataLen, divLen);
    printf("After adding remainder:\n");
    printData((unsigned char *)receivedWithCRC, dataLen + divLen - 1);

    // Check if the CRC part (remainder) is all zeros (ASCII '0')
    for (int i = dataLen; i < dataLen + divLen - 1; ++i) {
        if (receivedWithCRC[i] != '0') {
            return false; // CRC check failed
        }
    }
    return true; // CRC check passed
}
