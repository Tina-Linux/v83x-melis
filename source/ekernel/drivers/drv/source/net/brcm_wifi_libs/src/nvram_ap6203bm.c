/*
 *  #AP6203BM_NVRAM_V1.0_20200305
 *  # Sample variables file for BCM943012 BU board
 */

const char wifi_nvram_image[] = {
    "NVRAMRev=$Rev: 351687 $"												"\x00"
    "sromrev=11"												"\x00"
    "etmode=0x11"												"\x00"
    "cckdigfilttype=4"												"\x00"
    "bphyscale=0x28"												"\x00"
    "boardflags3=0x40000101"												"\x00"
    "vendid=0x14e4"												"\x00"
    "devid=0xA804"												"\x00"
    "manfid=0x2d0"												"\x00"
    "prodid=0x052e"												"\x00"
    "macaddr=00:90:4c:c5:12:38"												"\x00"
    "nocrc=1"												"\x00"
    "boardtype=0x080e"												"\x00"
    "boardrev=0x1103"												"\x00"
    "lpflags=0x00000020"												"\x00"
    "xtalfreq=37400"												"\x00"
    "boardflags2=0xc0800000"												"\x00"
    "boardflags=0x00400001"												"\x00"
    "extpagain2g=2"												"\x00"
    "extpagain5g=2"												"\x00"
    "ccode=0 "												"\x00"
    "regrev=0 "												"\x00"
    "antswitch = 0"												"\x00"
    "rxgains2gelnagaina0=0"												"\x00"
    "rxgains2gtrisoa0=15"												"\x00"
    "rxgains2gtrelnabypa0=0"												"\x00"
    "rxgains5gelnagaina0=0"												"\x00"
    "rxgains5gtrisoa0=9"												"\x00"
    "rxgains5gtrelnabypa0=0"												"\x00"
    "pdgain5g=0"												"\x00"
    "pdgain2g=0"												"\x00"
    "tworangetssi2g=0"												"\x00"
    "tworangetssi5g=0"												"\x00"
    "rxchain=1"												"\x00"
    "txchain=1"												"\x00"
    "aa2g=1"												"\x00"
    "aa5g=1"												"\x00"
    "tssipos5g=0"												"\x00"
    "tssipos2g=0"												"\x00"
    "tssisleep_en=0x5"												"\x00"
    "femctrl=17"												"\x00"
    "subband5gver=4"												"\x00"

    //"pa2ga0=-102,4805,-583"												"\x00"
    //"pa2ga0=-137,5218,-633"												"\x00"
    //"pa2ga0=-166,5339,-645"												"\x00"
    //"pa2ga0=-172,5143,-642"												"\x00"
    "pa2ga0=-152,4954,-619"												"\x00"
    //"pa2ga0=-135,5777,-677"												"\x00"

    //"pa5ga0=-194,5367,-658,-188,5286,-649,-166,5358,-653,-138,5140,-626"												"\x00"
    //"pa5ga0=-121,5285,-640,-113,5338,-646,-130,5574,-673,-160,5405,-665"												"\x00"
    //"pa5ga0=-175,5263,-641,-165,5229,-633,-140,5294,-630,-135,5255,-625"												"\x00"
    //"pa5ga0=-180,5167,-636,-170,5163,-634,-146,5281,-636,-143,5343,-638"												"\x00"
    //"pa5ga0=-152,5161,-631,-145,5181,-632,-128,5272,-640,-137,5228,-638"												"\x00"
    //"pa5ga0=-144,5209,-628,-135,5243,-631,-126,5280,-639,-124,5343,-644"												"\x00"
    "pa5ga0=-136,5564,-652,-130,4746,-600,-100,5599,-662,-99,5947,-675"												"\x00"
    //"pa5ga0=-164,5243,-632,-161,5228,-632,-139,5332,-637,-141,5307,-640"												"\x00"


    "cckpwroffset0=2"												"\x00"
    "pdoffset40ma0=0"												"\x00"
    "pdoffset80ma0=0"												"\x00"
    "lowpowerrange2g=0"												"\x00"
    "lowpowerrange5g=0"												"\x00"
    "ed_thresh2g=-63"												"\x00"
    "ed_thresh5g=-63"												"\x00"
    //"swctrlmap_2g=0x00000000,0x00400040, 0x00400040,0x000000,0x3e7"												"\x00"
    //"swctrlmapext_2g=0x00020002,0x00000000, 0x00000000,0x000000,0x003"												"\x00"
    //"swctrlmap_5g=0x00000000,0x00000000,0x00000000,0x000000,0x3a7"												"\x00"
    //"swctrlmapext_5g=0x00000000,0x00010001, 0x00010001,0x000000,0x001"												"\x00"
    "swctrlmap_2g=0x00000000,0x00400040, 0x00400040,0x204040,0x3e7"												"\x00"
    "swctrlmapext_2g=0x00020002,0x00000000, 0x00000000,0x000000,0x003"												"\x00"
    "swctrlmap_5g=0x00000000,0x00800080,0x00800080,0x000000,0x387"												"\x00"
    "swctrlmapext_5g=0x00010001,0x00000000, 0x00000000,0x000000,0x003"												"\x00"
    "ulpnap=0"												"\x00"
    "ulpadc=1"												"\x00"
    "ssagc_en=0"												"\x00"
    "ds1_nap=0"												"\x00"
    "epacal2g=0"												"\x00"
    "epacal5g=0"												"\x00"
    "epacal2g_mask=0x3fff"												"\x00"

    "maxp2ga0=77"												"\x00"
    "cckbw202gpo=0x4411"												"\x00"
    "ofdmlrbw202gpo=0x0011"												"\x00"
    "dot11agofdmhrbw202gpo=0x5211"												"\x00"
    "mcsbw202gpo=0x99843333"												"\x00"
    "mac_clkgating=1"												"\x00"
    //"mcsbw402gpo=0x99555533"												"\x00"

    "maxp5ga0=72,72,72,72"												"\x00"
    "mcsbw205glpo=0x99544441"												"\x00"
    "mcsbw205gmpo=0x99544441"												"\x00"
    "mcsbw205ghpo=0x99544441"												"\x00"

    //"mcsbw405glpo=0x99555000"												"\x00"
    //"mcsbw405gmpo=0x99555000"												"\x00"
    //"mcsbw405ghpo=0x99555000"												"\x00"
    //"mcsbw805glpo=0x99555000"												"\x00"
    //"mcsbw805gmpo=0x99555000"												"\x00"
    //"mcsbw805ghpo=0x99555000"												"\x00"

    "txwbpapden=1"												"\x00"
    "femctrlwar=0"												"\x00"
    "use5gpllfor2g=1"												"\x00"

    //"tx papd cal params"												"\x00"
    //"params are - 0x5g2g "												"\x00"
    "wb_rxattn=0x0303"												"\x00"
    "wb_txattn=0x0203"												"\x00"
    "wb_papdcalidx=0x1C05"												"\x00"
    "wb_eps_offset=0x01c101ad"												"\x00"
    "wb_bbmult=0x3C50"												"\x00"
    "wb_calref_db=0x1926"												"\x00"
    "wb_tia_gain_mode=0x0606"												"\x00"
    "wb_txbuf_offset=0x2020"												"\x00"
    "wb_frac_del=0x60b4"												"\x00"
    "wb_g_frac_bits=0xBA"												"\x00"

    "nb_rxattn=0x0403"												"\x00"
    "nb_txattn=0x0402"												"\x00"
    "nb_papdcalidx= 0x1405"												"\x00"
    "nb_eps_offset= 0x01d701ca"												"\x00"
    "nb_bbmult= 0x5A50"												"\x00"
    "nb_tia_gain_mode=0x0006"												"\x00"
    "AvVmid_c0=6,104,7,80,7,80,7,80,7,80"												"\x00"

    "lpo_select=4"												"\x00"

    "csml=0x10"												"\x00"
    "pt5db_gaintbl=0"												"\x00"

    "papdcck=0"												"\x00"
    "phycal_tempdelta=15"												"\x00"
    "ofdmfilttype_2gbe=1"												"\x00"

    "wowl_gpio=1"                                                           "\x00"
    "wowl_gpiopol=0"                                                        "\x00"

    //"deadman_to=1"												"\x00"
    "btc_mode=1"												"\x00"
    "muxenab=0x1"												"\x00"
    "\x00\x00"
};

char *wifi_nvram_ptr = (char *)wifi_nvram_image;
int wifi_nvram_size = (int)sizeof(wifi_nvram_image);
