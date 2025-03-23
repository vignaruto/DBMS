#include "bits/stdc++.h"
#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
using namespace std;

//stage-1

/*int main(int argc, char *argv[]) {
	/* Initialize the Run Copy of Disk */
	/*Disk disk_run;
	// StaticBuffer buffer;
	// OpenRelTable cache;
	unsigned char buffer[BLOCK_SIZE];
	Disk::readBlock(buffer,6000);
	char message[]="hello";
	memcpy(buffer,message,6);
	Disk::writeBlock(buffer,6000);
	unsigned char buffer2[BLOCK_SIZE];
	Disk::readBlock(buffer2,6000);
	char message2[6];
	memcpy(message2,buffer2,6);
	cout<<message2<<endl;
  //ex-1;
	/*int j;
	unsigned char k[101];
	unsigned char bu[BLOCK_SIZE];
	Disk::readBlock(bu,0);
	memcpy(k,bu,100);
	for(int i=0;i<15;i++){
		j=(int)k[i];
		cout<<j<<" ";
	}*/
	/*cout<<endl;
	return 0;
	//return FrontendInterface::handleFrontend(argc, argv);
}*/

//stage 2

// int main(int argc, char *argv[]) {
//   Disk disk_run;

//   // create objects for the relation catalog and attribute catalog
//   RecBuffer relCatBuffer(RELCAT_BLOCK);
//   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   HeadInfo relCatHeader;
//   HeadInfo attrCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader.
//   // (we will implement these functions later)
//   relCatBuffer.getHeader(&relCatHeader);
//   attrCatBuffer.getHeader(&attrCatHeader);

//   for (int i=0;i<relCatHeader.numEntries;i++/* i = 0 to total relation count */) {

//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

//     for (int j=0;j<attrCatHeader.numEntries;j++/* j = 0 to number of entries in the attribute catalog */) {
// 	Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
// 	attrCatBuffer.getRecord(attrCatRecord,j);
//       // declare attrCatRecord and load the attribute catalog entry into it

//       if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0/* attribute catalog entry corresponds to the current relation */) {
//         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
//         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal/* get the attribute name */, attrType);
//       }
//     }
//     printf("\n");
//   }

//   return 0;
// }

/* ex-1 and ex-2*/
// int main(int argc, char *argv[]) {

//   Disk disk_run;

//   // create objects for the relation catalog and attribute catalog
//   RecBuffer relCatBuffer(RELCAT_BLOCK);
//   RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   HeadInfo relCatHeader;
//   HeadInfo attrCatHeader;

//   // load the headers of both the blocks into relCatHeader and attrCatHeader.
//   // (we will implement these functions later)
//   relCatBuffer.getHeader(&relCatHeader);
//   attrCatBuffer.getHeader(&attrCatHeader);
//   unsigned char buffer[BLOCK_SIZE];
//   unsigned char b[16]="Batch";
//   int a;
//   for (int i=0;i<relCatHeader.numEntries;i++/* i = 0 to total relation count */) {
//     int flag=5;
//     Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//     relCatBuffer.getRecord(relCatRecord, i);

//     printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
//     while(flag!=-1){
//       HeadInfo x;
//       RecBuffer z(flag);
//       z.getHeader(&x);
//       flag=x.rblock;
//       for (int j=0;j<x.numEntries;j++/* j = 0 to number of entries in the attribute catalog */) {
//         Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
//         z.getRecord(attrCatRecord,j);
//             // declare attrCatRecord and load the attribute catalog entry into it

//         if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0/* attribute catalog entry corresponds to the current relation */) {
//           const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";

//           if(strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,"Class")==0){
//             Disk::readBlock(buffer,5);
//             memcpy(buffer+52+j*96+16,b,16);
//             Disk::writeBlock(buffer,5);
//             attrCatBuffer.getRecord(attrCatRecord,j);
//           }

//           printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal/* get the attribute name */, attrType);
//         }
//       }
//       printf("\n");
//   }
//   }

//   return 0;
// }

// stage-3
// int main(int argc,char *argv[]) {

//   Disk disk_run;
//   StaticBuffer buffer;
//   OpenRelTable cache;

//   // create objects for the relation catalog and attribute catalog
//   // RecBuffer relCatBuffer(RELCAT_BLOCK);
//   // RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

//   // HeadInfo relCatHeader;
//   // HeadInfo attrCatHeader;

//   // // load the headers of both the blocks into relCatHeader and attrCatHeader.
//   // // (we will implement these functions later)
//   // relCatBuffer.getHeader(&relCatHeader);
//   // attrCatBuffer.getHeader(&attrCatHeader);
//   // int a;
//   // for (int i=0;i<relCatHeader.numEntries;i++) {
//   //   int flag=5;
//   //   Attribute relCatRecord[RELCAT_NO_ATTRS]; // will store the record from the relation catalog

//   //   relCatBuffer.getRecord(relCatRecord, i);

//   //   printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);
//   //   while(flag!=-1){
//   //     HeadInfo x;
//   //     RecBuffer z(flag);
//   //     z.getHeader(&x);
//   //     flag=x.rblock;
//   //     for (int j=0;j<x.numEntries;j++/* j = 0 to number of entries in the attribute catalog */) {
//   //       Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
//   //       z.getRecord(attrCatRecord,j);
//   //           // declare attrCatRecord and load the attribute catalog entry into it

//   //       if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0/* attribute catalog entry corresponds to the current relation */) {
//   //         const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";


//   //         printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal/* get the attribute name */, attrType);
//   //       }
//   //     }
//   //     printf("\n");
//   // }
//   // }
//   for(int i=0;i<=2;i++){
//     RelCatEntry relCatEntry;
//     RelCacheTable::getRelCatEntry(i,&relCatEntry);
//     printf("Relation:%s\n",relCatEntry.relName);
//     for(int j=0;j<relCatEntry.numAttrs;j++){
//       AttrCatEntry attrcatentry;
//       AttrCacheTable::getAttrCatEntry(i,j,&attrcatentry);
//       const char *attrtype=attrcatentry.attrType==NUMBER?"NUM":"STR";
//       printf(" %s: %s\n",attrcatentry.attrName,attrtype);
//     }
//   }
//    return 0;
  
//  }


int main(int argc, char *argv[]) {
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;

  return FrontendInterface::handleFrontend(argc, argv);
}

