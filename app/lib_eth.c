//*****************************************************************************
//
// lib_eth.h - Ethernet routines for the IPMI application.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_ethernet.h"
#include "driverlib/ethernet.h"
#include "driverlib/interrupt.h"
#include "driverlib/flash.h"
#include "app/lib_eth.h"
#include "ipmi_lib/ipmi_modules.h"
#include "ipmi_lib/ipmi_cfg.h"
#include "third_party/uip-1.0/uip/uip.h"
#include "third_party/uip-1.0/uip/uip_arp.h"

//*****************************************************************************
//
// The Address of Hardware and Internet.
//
//*****************************************************************************
uip_ipaddr_t ipaddr;
struct uip_eth_addr macaddr;

//*****************************************************************************
//
// The interrupt handler for the eth0 interrupt.
//
//*****************************************************************************
void ETH_eth0_int_handler(void)
{
    unsigned long ulTemp;

    //
    // Read and Clear the interrupt.
    //
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);

    //
    // Check to see if an RX Interrupt has occured.
    //
    if(ulTemp & ETH_INT_RX)
    {
        //
        // Indicate that a packet has been received.
        //
        //HWREGBITW(&g_ulFlags, FLAG_RXPKT) = 1;

        //
        // Disable Ethernet RX Interrupt.
        //
        EthernetIntDisable(ETH_BASE, ETH_INT_RX);
    }
}
#if 0
//*****************************************************************************
//
// The ETH0 interrupt handler.
//
//*****************************************************************************
void ETH0IntHandler(void)
{
    ipmi_intf_recv_post(IPMI_INTF_ETH);
}
#endif

//*****************************************************************************
//
// Init ethernet hardware to REMOTE as Lan Interface
//
//*****************************************************************************
void ETH_eth0_init(void)
{
    unsigned long ulTemp;
    unsigned long ulUser0, ulUser1;

    //
    // Enable and Reset the Ethernet Controller.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    //
    // Enable Port F for Ethernet LEDs.
    //  LED0        Bit 3   Output
    //  LED1        Bit 2   Output
    //
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    //
    // Intialize the Ethernet Controller and disable all Ethernet Controller
    // interrupt sources.
    //
    EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                       ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);

    //
    // Initialize the Ethernet Controller for operation.
    //
    EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

    //
    // Configure the Ethernet Controller for normal operation.
    // - Full Duplex
    // - TX CRC Auto Generation
    // - TX Padding Enabled
    //
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                                 ETH_CFG_TX_PADEN));

    //
    // Wait for the link to become active.
    //
    //RIT128x96x4StringDraw("Waiting for Link", 12, 8, 15);
    /*
    ulTemp = EthernetPHYRead(ETH_BASE, PHY_MR1);
    while((ulTemp & 0x0004) == 0)
    {
        ulTemp = EthernetPHYRead(ETH_BASE, PHY_MR1);
    }
    */
    //RIT128x96x4StringDraw("Link Established", 12, 16, 15);

    //
    // Enable the Ethernet Controller.
    //
    EthernetEnable(ETH_BASE);

    //
    // Enable the Ethernet interrupt.
    //
    IntEnable(INT_ETH);

    //
    // Enable the Ethernet RX Packet interrupt source.
    //
    EthernetIntEnable(ETH_BASE, ETH_INT_RX);

    //
    // Initialize the uIP TCP/IP stack.
    //
    uip_init();

#ifdef USE_STATIC_IP
    uip_ipaddr(ipaddr, DEFAULT_IPADDR0, DEFAULT_IPADDR1, DEFAULT_IPADDR2,
               DEFAULT_IPADDR3);
    uip_sethostaddr(ipaddr);
    //DisplayIPAddress(ipaddr, 18, 24);
    uip_ipaddr(ipaddr, DEFAULT_NETMASK0, DEFAULT_NETMASK1, DEFAULT_NETMASK2,
               DEFAULT_NETMASK3);
    uip_setnetmask(ipaddr);
#else
    uip_ipaddr(ipaddr, 0, 0, 0, 0);
    uip_sethostaddr(ipaddr);
    //DisplayIPAddress(ipaddr, 18, 24);
    uip_ipaddr(ipaddr, 0, 0, 0, 0);
    uip_setnetmask(ipaddr);
#endif

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the Ethernet Eval Kits, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        //
        //RIT128x96x4StringDraw("MAC Address", 0, 16, 15);
        //RIT128x96x4StringDraw("Not Programmed!", 0, 24, 15);
        FlashUserSet(DEFAULT_MACADDR0, DEFAULT_MACADDR1);
        FlashUserSave();
        ulUser0 = DEFAULT_MACADDR0;
        ulUser1 = DEFAULT_MACADDR1;
        //while(1) {}
    }

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    macaddr.addr[0] = ((ulUser0 >>  0) & 0xff);
    macaddr.addr[1] = ((ulUser0 >>  8) & 0xff);
    macaddr.addr[2] = ((ulUser0 >> 16) & 0xff);
    macaddr.addr[3] = ((ulUser1 >>  0) & 0xff);
    macaddr.addr[4] = ((ulUser1 >>  8) & 0xff);
    macaddr.addr[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Program the hardware with it's MAC address (for filtering).
    //
    EthernetMACAddrSet(ETH_BASE, (unsigned char *)&macaddr);
    uip_setethaddr(macaddr);

    // TODO
    //OSTaskCreate
}



//*****************************************************************************
//
// Init the Ethernet Interface
//
//*****************************************************************************
void ETH_init(void)
{
#if (defined(IPMI_MODULES_ETH_LAN))
    ETH_eth0_init();
#endif
}

