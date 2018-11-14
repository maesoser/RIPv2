#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

void print_help();

int main(int argc, char **argv)
{
    int fd;
    struct ifreq ifr;
    char *iface = "wlp2s0";
    FILE *fp;

    int opt;
    while ((opt = getopt(argc, argv, "i:h")) != -1) {
      switch(opt) {
          case 'i':
            iface = optarg;
            break;
          case 'h':
    	    print_help();
    	    break;
      }
    }
    if(iface==NULL){
      printf("Please, insert an interface.\n");
      print_help();
    }

	  fp = fopen("config.txt", "w+");

    fprintf(fp, "Interface %s\n", iface);

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;	    //Type of address to retrieve - IPv4 IP address
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);	    //Copy the interface name in the ifreq structure

    //get the ip address
    ioctl(fd, SIOCGIFADDR, &ifr);

    printf("Interface:\t%s\n",iface);
    //display ip
    printf("Address:\t%s\n" , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
    fprintf(fp, "IPv4Address %s\n", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));

    //get the netmask ip
    ioctl(fd, SIOCGIFNETMASK, &ifr);

    //display netmask
    printf("Netmask:\t%s\n" , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
    fprintf(fp, "SubnetMask %s\n", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));

    close(fd);

    /*
     *
     * Interface eth0
		IPv4Address 163.117.144.127
		SubnetMask 255.255.255.0
	*/

   fclose(fp);

    return 0;
}

void print_help(){
  printf("aconf - generate automatically the IP config file for the selected interface.\n");
  printf("Use:\n");
  printf("\t./aconf -i [IF]\tGenerate config file\n");
  printf("\t./aconf -h\tShow this help.\n");
  exit(0);
}
