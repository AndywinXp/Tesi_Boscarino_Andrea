#include "dummy.h"

void start_simulation()
{
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    WSADATA wsa;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialised.\n");

    //create socket
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        printf("socket() failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    //setup address structure
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

    BYTE *frame_pipeline    = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));
    BYTE *frame_luma        = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));
	BYTE *frame_chroma      = (BYTE*)malloc(WIDTH*HEIGHT*sizeof(BYTE));

    packet_data* dummy = (packet_data*) malloc(sizeof(packet_data));
    dummy->count = 0;
    dummy->fragment = 0;
    dummy->frame_index = 0;

    memcpy(dummy->data, frame_pipeline, sizeof(dummy->data));
    memcpy(dummy->data_luma, frame_luma, sizeof(dummy->data_luma));
    memcpy(dummy->data_chroma, frame_chroma, sizeof(dummy->data_chroma));

    int image_index = 0;
    IplImage* loadedZynq;
    IplImage* loadedChroma;
    IplImage* loadedLuma;

    char currentNumber[BUFFER_SIZE];
    char currentStandard[BUFFER_SIZE];
    char currentChroma[BUFFER_SIZE];
    char currentLuma[BUFFER_SIZE];
    //start communication
    while(1)
    {
        // flushing the strings
        strncpy(currentNumber, "", sizeof(currentNumber));
        strncpy(currentStandard, "", sizeof(currentStandard));
        strncpy(currentChroma, "", sizeof(currentChroma));
        strncpy(currentLuma, "", sizeof(currentLuma));

        // setting up filenames
        sprintf(currentNumber, "%d.png", image_index);

        strcat(currentStandard, "ZYNQ_");
        strcat(currentLuma, "LUMA_");
        strcat(currentChroma, "CHROMA_");

        strcat(currentStandard, currentNumber);
        strcat(currentLuma, currentNumber);
        strcat(currentChroma, currentNumber);

        printf("Attempting to load frame number %d from files\n", image_index);
        //sleep(1);

        // ZYNQ LOADING ATTEMPT
        if ((loadedZynq = cvLoadImage(currentStandard, CV_LOAD_IMAGE_ANYDEPTH)) == NULL) {
            printf("Couldn't load file %s, sorry.\n", currentStandard);
            return -1;
        } else
            printf("Canvas size for %s %dx%d\n", currentStandard, loadedZynq->width, loadedZynq->height);

            //cvShowImage("TEST1", loadedZynq); //Testing purpose
            //cvWaitKey(16);
        // LUMA LOADING ATTEMPT
        if ((loadedLuma = cvLoadImage(currentLuma, CV_LOAD_IMAGE_ANYDEPTH)) == NULL) {
            printf("Couldn't load file %s, sorry.\n", currentLuma);
            return -1;
        } else
            printf("Canvas size for %s %dx%d\n", currentLuma, loadedZynq->width, loadedZynq->height);

            //cvShowImage("TEST2", loadedLuma); //Testing purpose
            //cvWaitKey(16);
        // CHROMA LOADING ATTEMPT
        if ((loadedChroma = cvLoadImage(currentChroma, CV_LOAD_IMAGE_ANYDEPTH)) == NULL) {
            printf("Couldn't load file %s, sorry.\n", currentChroma);
            return -1;
        } else
            printf("Canvas size for %s %dx%d\n", currentChroma, loadedZynq->width, loadedZynq->height);

            //cvShowImage("TEST3", loadedChroma); //Testing purpose

        // REFRESH THE SCREEN!
        cvWaitKey(16);

        // Invio dei frammenti:
        // pacchettizzeremo FRAME_I_SIZE alla volta, scorrendo lungo il singolo frame
        // e li invieremo man mano.
        // Prepare the message
        int i = 0;
        for (i=0; i<(FRAME_SIZE/FRAME_I_SIZE); i++) {
            memcpy(dummy->data,        loadedZynq->imageData   + i*FRAME_I_SIZE, sizeof(dummy->data));
            memcpy(dummy->data_luma,   loadedLuma->imageData   + i*FRAME_I_SIZE, sizeof(dummy->data_luma));
            memcpy(dummy->data_chroma, loadedChroma->imageData + i*FRAME_I_SIZE, sizeof(dummy->data_luma));

            // invio pacchetto col frammento
            if (sendto(s, dummy, sizeof(packet_data) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
            {
                printf("sendto() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            }
            printf("Frammento inviato numero %d\n", dummy->fragment);
            //sleep(1);
            dummy->count++;
            dummy->fragment == FRAME_SIZE/FRAME_I_SIZE ? dummy->fragment = 0 : dummy->fragment++;

        }
        dummy->frame_index++;
        dummy->fragment = 0;
        printf("Frame inviato: immagine %d\n", image_index);
        usleep(10000);
        image_index == MAX_IMAGE_NUM ? image_index = 0 : image_index++;
    }

    closesocket(s);
    WSACleanup();
}
