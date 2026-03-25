#include "UAVDataTransmition.h"
#include <QtWidgets/QApplication>
#include<QSslConfiguration>°°
#include<iostream>
//void tc9_test_nvm_short(void);
int main(int argc, char *argv[])
{

	QApplication a(argc, argv);

	QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
	sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
	QSslConfiguration::setDefaultConfiguration(sslConf);

	UAVDataTransmition w;
	w.show();
  /*  tc9_test_nvm_short();*/
	return a.exec();
}
//void tc9_test_nvm_short(void)
//{
//    uint32_t *nvm_base_addr = new uint32_t;//0x64000000;
//    uint32_t nvm_size = 512 * 1024; //bytes
//    uint32_t errctr = 0;
//    uint32_t index = 0;
//    uint32_t data[2] = { 56.754, 0xAA55 };
//    int8_t last = -1;
//    uint16_t* ptr;
//
//   // nvmInit();
//    ptr = (uint16_t*)(nvm_base_addr);
//   // sciPrintf("\r\nNVM test####################################\r\n");
//
//    last = 0;
//   // _rti_delay_ms(DELAY_TIME_MS);
//   printf("write data: %x\r\n", data[last]);
//    //–¥»Î
//    errctr = 0;
//    for (index = 0; index < nvm_size; index++)
//    {
//        *(ptr + index) = data[last];
//    }
//   /* _rti_delay_ms(DELAY_TIME_MS);*/
//    //        sciPrintf("write ok. start read back.\r\n");
//        //¡¢º¥ªÿ∂¡
//    for (index = 0; index < nvm_size; index++)
//    {
//        if (*(ptr + index) != data[last])
//        {
//            errctr++;
//        }
//    }
//}