#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
 
#define BUFFER_LENGTH 256
 
int main(){
   const char* path = "/dev/phonebook_device";
   int ret, fd;
   char receive[BUFFER_LENGTH];
   const char* stringsToSend[] = {
      "[add]name: Ivan\nsurname: Brackman\nage: 26\nphone: 555-35-35\nemail: ivan.brackman@gmail.com\n", 
      "[get]Brackman",
      "[del]Ivan",
      "[del]Brackman",
      "[get]Brackman",
      "[add]name: Ivan\nsurname: Brackman\nage: 26\nphone: 555-35-35\nemail: ivan.brackman@gmail.com\n",
      "[add]name: Ivan\nsurname: Putin\nage: 99\nphone: 01\nemail: mail@example.com\n",
      "[get]Brackman",
      "[get]Putin",
      "[teg]Putin",
      "[del]Putin",
      "[del]Brackman"
      };

   fd = open("/dev/phonebook_device", O_RDWR);
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   for (int i = 0; i < sizeof(stringsToSend) / sizeof(stringsToSend[0]); ++i) {
      memset(receive, 0, 256);
      printf("Writing message to the device:\n%s\n", stringsToSend[i]);
      ret = write(fd, stringsToSend[i], strlen(stringsToSend[i])); 
      if (ret < 0){
         puts("Failed to write the message to the device.");
         continue;
      }
    
      ret = read(fd, receive, BUFFER_LENGTH);
      if (ret < 0){
         puts("Failed to read the message from the device.");
         continue;
      }
      printf("The received message is:\n%s\n", receive);
   }
   printf("End of the program\n");
   return 0;
}