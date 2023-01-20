#created by gcc template
path=C:\Windows;C:\Windows\system32;C:\PaxPayPro2017\build\gcc410\bin

NAME      = ARCAPAY
GCCDIR    = C:\PaxPayPro2017\build\gcc410
GCCBIN    = $(GCCDIR)\bin
GCCLIB    = -L. -L$(GCCDIR)\lib
SRCDIR    = C:\SPOUT\PP\ARCAPAY\Source
BASEDIR	  = C:\SPOUT\PP\ARCAPAY
LOCOBJ    = C:\SPOUT\PP\ARCAPAY\obj_Sxx
LOCINC    = -I. -IC:\PaxPayPro2017\include
POSLIBDIR = C:\PaxPayPro2017\postype\Sxx
POSLIB    = s80api

ASM       = $(GCCBIN)\arm-elf-gcc -Os -msoft-float -mlittle-endian -ffunction-sections -D_S80_POS $(LOCINC) -c 
GCC       = $(GCCBIN)\arm-elf-gcc -Os -msoft-float -mlittle-endian -ffunction-sections -D_S80_POS $(LOCINC) -c
LINK      = $(GCCBIN)\arm-elf-ld -T$(POSLIBDIR)\sldscript_s80 -Map $(LOCOBJ)\$(NAME).map --gc-sections --cref -nostartfiles $(GCCLIB) -L$(POSLIBDIR) -MAP > filename.map 
OBJ       = $(LOCOBJ)\init.o \
			$(LOCOBJ)\CppInit.o \
            $(LOCOBJ)\appLib.o \
            $(LOCOBJ)\BmpToPrnLogo.o \
            $(LOCOBJ)\checkopt.o \
			$(LOCOBJ)\callhome.o \
            $(LOCOBJ)\ClssProc.o \
            $(LOCOBJ)\commlib.o \
            $(LOCOBJ)\commproc.o \
            $(LOCOBJ)\cpinpad.o \
            $(LOCOBJ)\debug.o \
			$(LOCOBJ)\dl_iso8583.o \
			$(LOCOBJ)\dl_iso8583_common.o \
			$(LOCOBJ)\dl_iso8583_defs_1987.o \
			$(LOCOBJ)\dl_iso8583_defs_1993.o \
			$(LOCOBJ)\dl_iso8583_fields.o \
			$(LOCOBJ)\dl_mem.o \
			$(LOCOBJ)\dl_output.o \
			$(LOCOBJ)\dl_str.o \
            $(LOCOBJ)\emvproc.o \
			$(LOCOBJ)\eod.o \
            $(LOCOBJ)\emvtest.o \
			$(LOCOBJ)\emvkeys.o \
            $(LOCOBJ)\eventmsg.o \
            $(LOCOBJ)\fileoper.o \
            $(LOCOBJ)\global.o \
            $(LOCOBJ)\GUI.o \
			$(LOCOBJ)\http.o \
            $(LOCOBJ)\initpos.o \
			$(LOCOBJ)\isoKeyExchange.o \
            $(LOCOBJ)\lng.o \
			$(LOCOBJ)\logger.o \
			$(LOCOBJ)\jsmn.o \
			$(LOCOBJ)\json.o \
            $(LOCOBJ)\main.o \
            $(LOCOBJ)\manage.o \
			$(LOCOBJ)\merchant.o \
            $(LOCOBJ)\mlogo.o \
            $(LOCOBJ)\MultiApp.o \
            $(LOCOBJ)\pack8583.o \
            $(LOCOBJ)\password.o \
            $(LOCOBJ)\print.o \
            $(LOCOBJ)\ProcTrans.o \
			$(LOCOBJ)\profile.o \
			$(LOCOBJ)\rc4.o \
			$(LOCOBJ)\receipt.o \
			$(LOCOBJ)\reprint.o \
			$(LOCOBJ)\sha256.o \
            $(LOCOBJ)\setup.o \
            $(LOCOBJ)\Simulate.o \
			$(LOCOBJ)\settings.o \
            $(LOCOBJ)\st8583.o \
            $(LOCOBJ)\SxxCom.o \
            $(LOCOBJ)\SxxComSSL.o \
            $(LOCOBJ)\tranproc.o \
            $(LOCOBJ)\util.o \
			$(LOCOBJ)\admin.o \
			$(LOCOBJ)\balance.o \
			$(LOCOBJ)\cashadvance.o \
			$(LOCOBJ)\cashback.o \
			$(LOCOBJ)\nfc.o \
			$(LOCOBJ)\preauth.o \
			$(LOCOBJ)\refund.o \
			$(LOCOBJ)\reversal.o \
			$(LOCOBJ)\completion.o \
			$(LOCOBJ)\utilities.o \
			$(LOCOBJ)\appdownload.o \
			$(LOCOBJ)\logodownload.o \
			$(LOCOBJ)\keysdownload.o \
			$(LOCOBJ)\http_handler.o \
			
#$(NAME).elf: $(OBJ) makefile
  #  $(LINK) -o $(LOCOBJ)\$(NAME).elf $(OBJ) --start-group "$(BASEDIR)\lib\B_DPAS_LIB_v100.a" "$(BASEDIR)\lib\B_ENTRY_LIB_v505.a" "$(BASEDIR)\lib\B_JCB_LIB_v101.a" "$(BASEDIR)\lib\B_WAVE_LIB_v305.a" "$(BASEDIR)\lib\C_ENTRY_LIB_v505.a" "$(BASEDIR)\lib\C_JCB_LIB_v101.a" "$(BASEDIR)\lib\C_WAVE_LIB_v305.a" "$(BASEDIR)\lib\Device_PaxMe_B_v503.a" "$(BASEDIR)\lib\libClAERSFMSF_v202.a" "$(BASEDIR)\lib\libClAESMFMNF_v202.a" "$(BASEDIR)\lib\libClQPBOCRSFMSF_v300.a" "$(BASEDIR)\lib\libCrypto_V1.2.a" "$(BASEDIR)\lib\LibemvSxxSF_v412.a" "$(BASEDIR)\lib\libsslSxx_Release.a" "$(BASEDIR)\lib\MC_PaxMe_B_v502.a" "$(BASEDIR)\lib\libzip_V2.2.a" "$(BASEDIR)\lib\libbarcode_paxme01.a" -l$(POSLIB) -lc -lgcc --end-group

$(NAME).elf: $(OBJ) makefile
    $(LINK) -o $(LOCOBJ)\$(NAME).elf $(OBJ) --start-group "$(BASEDIR)\lib\B_DPAS_LIB_v100.a" "$(BASEDIR)\lib\B_ENTRY_LIB_v505.a" "$(BASEDIR)\lib\B_JCB_LIB_v101.a" "$(BASEDIR)\lib\B_WAVE_LIB_v305.a" "$(BASEDIR)\lib\C_ENTRY_LIB_v505.a" "$(BASEDIR)\lib\C_JCB_LIB_v101.a" "$(BASEDIR)\lib\C_WAVE_LIB_v305.a" "$(BASEDIR)\lib\Device_PaxMe_B_v503.a" "$(BASEDIR)\lib\libClAERSFMSF_v202.a" "$(BASEDIR)\lib\libClAESMFMNF_v202.a" "$(BASEDIR)\lib\libClQPBOCRSFMSF_v300.a" "$(BASEDIR)\lib\LibemvSxxSF_v412.a" "$(BASEDIR)\lib\MC_PaxMe_B_v502.a" "$(BASEDIR)\lib\libzip_V2.2.a" "$(BASEDIR)\lib\libbarcode_paxme01.a" "$(BASEDIR)\lib\libPaxSSL_V3.0.7-190129-Sxx.a" "$(BASEDIR)\lib\libhttpSxx_V1.0.3_Release.a" "$(BASEDIR)\lib\libCrypto_V3.0.7-190129-Sxx.a" -l$(POSLIB) -lc -lgcc --end-group


#	arm-elf-readelf -a $(LOCOBJ)\$(NAME).elf > $(LOCOBJ)\$(NAME).sys
#	arm-elf-objdump -D -S $(LOCOBJ)\$(NAME).elf > $(LOCOBJ)\$(NAME).dasm
    elftobin $(LOCOBJ)\$(NAME).elf $(BASEDIR)\Send\$(NAME).bin PAX-S80-SAPP

	
# ASM file
$(LOCOBJ)\init.o: "$(POSLIBDIR)\init_s80.s"
        $(ASM) "$(POSLIBDIR)\init_s80.s" -o "$(LOCOBJ)\init.o"

# init file
$(LOCOBJ)\CppInit.o: "$(GCCDIR)\lib\CppInit.c"
        $(GCC) "$(GCCDIR)\lib\CppInit.c -o "$(LOCOBJ)\CppInit.o"
		
#  C files
$(LOCOBJ)\appLib.o: $(SRCDIR)\appLib.c
        $(GCC) $(SRCDIR)\appLib.c -o "$(LOCOBJ)\appLib.o"
$(LOCOBJ)\BmpToPrnLogo.o: $(SRCDIR)\BmpToPrnLogo.c
        $(GCC) $(SRCDIR)\BmpToPrnLogo.c -o "$(LOCOBJ)\BmpToPrnLogo.o"
$(LOCOBJ)\callhome.o: $(SRCDIR)\callhome.c
        $(GCC) $(SRCDIR)\callhome.c -o "$(LOCOBJ)\callhome.o"
$(LOCOBJ)\checkopt.o: $(SRCDIR)\checkopt.c
        $(GCC) $(SRCDIR)\checkopt.c -o "$(LOCOBJ)\checkopt.o"
$(LOCOBJ)\ClssProc.o: $(SRCDIR)\ClssProc.c
        $(GCC) $(SRCDIR)\ClssProc.c -o "$(LOCOBJ)\ClssProc.o"
$(LOCOBJ)\commlib.o: $(SRCDIR)\commlib.c
        $(GCC) $(SRCDIR)\commlib.c -o "$(LOCOBJ)\commlib.o"
$(LOCOBJ)\commproc.o: $(SRCDIR)\commproc.c
        $(GCC) $(SRCDIR)\commproc.c -o "$(LOCOBJ)\commproc.o"
$(LOCOBJ)\cpinpad.o: $(SRCDIR)\cpinpad.c
        $(GCC) $(SRCDIR)\cpinpad.c -o "$(LOCOBJ)\cpinpad.o"
$(LOCOBJ)\debug.o: $(SRCDIR)\debug.c
        $(GCC) $(SRCDIR)\debug.c -o "$(LOCOBJ)\debug.o"
$(LOCOBJ)\dl_iso8583.o: $(SRCDIR)\dliso\dl_iso8583.c
        $(GCC) $(SRCDIR)\dliso\dl_iso8583.c -o $(LOCOBJ)\dl_iso8583.o
$(LOCOBJ)\dl_iso8583_common.o: $(SRCDIR)\dliso\dl_iso8583_common.c
        $(GCC) $(SRCDIR)\dliso\dl_iso8583_common.c -o $(LOCOBJ)\dl_iso8583_common.o
$(LOCOBJ)\dl_iso8583_defs_1987.o: $(SRCDIR)\dliso\dl_iso8583_defs_1987.c
        $(GCC) $(SRCDIR)\dliso\dl_iso8583_defs_1987.c -o $(LOCOBJ)\dl_iso8583_defs_1987.o
$(LOCOBJ)\dl_iso8583_defs_1993.o: $(SRCDIR)\dliso\dl_iso8583_defs_1993.c
        $(GCC) $(SRCDIR)\dliso\dl_iso8583_defs_1993.c -o $(LOCOBJ)\dl_iso8583_defs_1993.o
$(LOCOBJ)\dl_iso8583_fields.o: $(SRCDIR)\dliso\dl_iso8583_fields.c
        $(GCC) $(SRCDIR)\dliso\dl_iso8583_fields.c -o $(LOCOBJ)\dl_iso8583_fields.o
$(LOCOBJ)\dl_mem.o: $(SRCDIR)\dliso\dl_mem.c
        $(GCC) $(SRCDIR)\dliso\dl_mem.c -o $(LOCOBJ)\dl_mem.o
$(LOCOBJ)\dl_output.o: $(SRCDIR)\dliso\dl_output.c
        $(GCC) $(SRCDIR)\dliso\dl_output.c -o $(LOCOBJ)\dl_output.o
$(LOCOBJ)\dl_str.o: $(SRCDIR)\dliso\dl_str.c
        $(GCC) $(SRCDIR)\dliso\dl_str.c -o $(LOCOBJ)\dl_str.o
$(LOCOBJ)\emvkeys.o: $(SRCDIR)\emvkeys.c
        $(GCC) $(SRCDIR)\emvkeys.c -o "$(LOCOBJ)\emvkeys.o"
$(LOCOBJ)\emvproc.o: $(SRCDIR)\emvproc.c
        $(GCC) $(SRCDIR)\emvproc.c -o "$(LOCOBJ)\emvproc.o"
$(LOCOBJ)\eod.o: $(SRCDIR)\eod.c
        $(GCC) $(SRCDIR)\eod.c -o "$(LOCOBJ)\eod.o"
$(LOCOBJ)\emvtest.o: $(SRCDIR)\emvtest.c
        $(GCC) $(SRCDIR)\emvtest.c -o "$(LOCOBJ)\emvtest.o"
$(LOCOBJ)\eventmsg.o: $(SRCDIR)\eventmsg.c
        $(GCC) $(SRCDIR)\eventmsg.c -o "$(LOCOBJ)\eventmsg.o"
$(LOCOBJ)\fileoper.o: $(SRCDIR)\fileoper.c
        $(GCC) $(SRCDIR)\fileoper.c -o "$(LOCOBJ)\fileoper.o"
$(LOCOBJ)\global.o: $(SRCDIR)\global.c
        $(GCC) $(SRCDIR)\global.c -o "$(LOCOBJ)\global.o"
$(LOCOBJ)\http.o: $(SRCDIR)\http.c
        $(GCC) $(SRCDIR)\http.c -o "$(LOCOBJ)\http.o"
$(LOCOBJ)\GUI.o: $(SRCDIR)\GUI.c
        $(GCC) $(SRCDIR)\GUI.c -o "$(LOCOBJ)\GUI.o"
$(LOCOBJ)\initpos.o: $(SRCDIR)\initpos.c
        $(GCC) $(SRCDIR)\initpos.c -o "$(LOCOBJ)\initpos.o"
$(LOCOBJ)\isoKeyExchange.o: $(SRCDIR)\isoKeyExchange.c
        $(GCC) $(SRCDIR)\isoKeyExchange.c -o "$(LOCOBJ)\isoKeyExchange.o"
$(LOCOBJ)\jsmn.o: $(SRCDIR)\jsmn.c
        $(GCC) $(SRCDIR)\jsmn.c -o "$(LOCOBJ)\jsmn.o"
$(LOCOBJ)\json.o: $(SRCDIR)\json.c
        $(GCC) $(SRCDIR)\json.c -o "$(LOCOBJ)\json.o"
$(LOCOBJ)\lng.o: $(SRCDIR)\lng.c
        $(GCC) $(SRCDIR)\lng.c -o "$(LOCOBJ)\lng.o"
$(LOCOBJ)\logger.o: $(SRCDIR)\logger.c
        $(GCC) $(SRCDIR)\logger.c -o "$(LOCOBJ)\logger.o"
$(LOCOBJ)\main.o: $(SRCDIR)\main.c
        $(GCC) $(SRCDIR)\main.c -o "$(LOCOBJ)\main.o"
$(LOCOBJ)\manage.o: $(SRCDIR)\manage.c
        $(GCC) $(SRCDIR)\manage.c -o "$(LOCOBJ)\manage.o"
$(LOCOBJ)\merchant.o: $(SRCDIR)\merchant.c
        $(GCC) $(SRCDIR)\merchant.c -o "$(LOCOBJ)\merchant.o"
$(LOCOBJ)\mlogo.o: $(SRCDIR)\mlogo.c
        $(GCC) $(SRCDIR)\mlogo.c -o "$(LOCOBJ)\mlogo.o"
$(LOCOBJ)\MultiApp.o: $(SRCDIR)\MultiApp.c
        $(GCC) $(SRCDIR)\MultiApp.c -o "$(LOCOBJ)\MultiApp.o"
$(LOCOBJ)\pack8583.o: $(SRCDIR)\pack8583.c
        $(GCC) $(SRCDIR)\pack8583.c -o "$(LOCOBJ)\pack8583.o"
$(LOCOBJ)\password.o: $(SRCDIR)\password.c
        $(GCC) $(SRCDIR)\password.c -o "$(LOCOBJ)\password.o"
$(LOCOBJ)\print.o: $(SRCDIR)\print.c
        $(GCC) $(SRCDIR)\print.c -o "$(LOCOBJ)\print.o"
$(LOCOBJ)\profile.o: $(SRCDIR)\profile.c
        $(GCC) $(SRCDIR)\profile.c -o "$(LOCOBJ)\profile.o"
$(LOCOBJ)\ProcTrans.o: $(SRCDIR)\ProcTrans.c
        $(GCC) $(SRCDIR)\ProcTrans.c -o "$(LOCOBJ)\ProcTrans.o"
$(LOCOBJ)\rc4.o: $(SRCDIR)\rc4.c
        $(GCC) $(SRCDIR)\rc4.c -o "$(LOCOBJ)\rc4.o"
$(LOCOBJ)\receipt.o: $(SRCDIR)\receipt.c
        $(GCC) $(SRCDIR)\receipt.c -o "$(LOCOBJ)\receipt.o"
$(LOCOBJ)\reprint.o: $(SRCDIR)\reprint.c
        $(GCC) $(SRCDIR)\reprint.c -o "$(LOCOBJ)\reprint.o"
$(LOCOBJ)\setup.o: $(SRCDIR)\setup.c
        $(GCC) $(SRCDIR)\setup.c -o "$(LOCOBJ)\setup.o"
$(LOCOBJ)\settings.o: $(SRCDIR)\settings.c
        $(GCC) $(SRCDIR)\settings.c -o "$(LOCOBJ)\settings.o"
$(LOCOBJ)\Simulate.o: $(SRCDIR)\Simulate.c
        $(GCC) $(SRCDIR)\Simulate.c -o "$(LOCOBJ)\Simulate.o"
$(LOCOBJ)\sha256.o: $(SRCDIR)\sha256.c
        $(GCC) $(SRCDIR)\sha256.c -o "$(LOCOBJ)\sha256.o"
$(LOCOBJ)\st8583.o: $(SRCDIR)\st8583.c
        $(GCC) $(SRCDIR)\st8583.c -o "$(LOCOBJ)\st8583.o"
$(LOCOBJ)\SxxCom.o: $(SRCDIR)\SxxCom.c
        $(GCC) $(SRCDIR)\SxxCom.c -o "$(LOCOBJ)\SxxCom.o"
$(LOCOBJ)\SxxComSSL.o: $(SRCDIR)\SxxComSSL.c
        $(GCC) $(SRCDIR)\SxxComSSL.c -o "$(LOCOBJ)\SxxComSSL.o"
$(LOCOBJ)\tranproc.o: $(SRCDIR)\tranproc.c
        $(GCC) $(SRCDIR)\tranproc.c -o "$(LOCOBJ)\tranproc.o"
$(LOCOBJ)\util.o: $(SRCDIR)\util.c
        $(GCC) $(SRCDIR)\util.c -o "$(LOCOBJ)\util.o"
$(LOCOBJ)\admin.o: $(SRCDIR)\admin.c
        $(GCC) $(SRCDIR)\admin.c -o "$(LOCOBJ)\admin.o"
$(LOCOBJ)\balance.o: $(SRCDIR)\balance.c
        $(GCC) $(SRCDIR)\balance.c -o "$(LOCOBJ)\balance.o"
$(LOCOBJ)\cashadvance.o: $(SRCDIR)\cashadvance.c
        $(GCC) $(SRCDIR)\cashadvance.c -o "$(LOCOBJ)\cashadvance.o"
$(LOCOBJ)\cashback.o: $(SRCDIR)\cashback.c
        $(GCC) $(SRCDIR)\cashback.c -o "$(LOCOBJ)\cashback.o"
$(LOCOBJ)\nfc.o: $(SRCDIR)\nfc.c
        $(GCC) $(SRCDIR)\nfc.c -o "$(LOCOBJ)\nfc.o"
$(LOCOBJ)\preauth.o: $(SRCDIR)\preauth.c
        $(GCC) $(SRCDIR)\preauth.c -o "$(LOCOBJ)\preauth.o"
$(LOCOBJ)\refund.o: $(SRCDIR)\refund.c
        $(GCC) $(SRCDIR)\refund.c -o "$(LOCOBJ)\refund.o"
$(LOCOBJ)\reversal.o: $(SRCDIR)\reversal.c
        $(GCC) $(SRCDIR)\reversal.c -o "$(LOCOBJ)\reversal.o"
$(LOCOBJ)\completion.o: $(SRCDIR)\completion.c
        $(GCC) $(SRCDIR)\completion.c -o "$(LOCOBJ)\completion.o"
$(LOCOBJ)\utilities.o: $(SRCDIR)\utilities.c
        $(GCC) $(SRCDIR)\utilities.c -o "$(LOCOBJ)\utilities.o"
$(LOCOBJ)\appdownload.o: $(SRCDIR)\appdownload.c
        $(GCC) $(SRCDIR)\appdownload.c -o "$(LOCOBJ)\appdownload.o"
$(LOCOBJ)\logodownload.o: $(SRCDIR)\logodownload.c
        $(GCC) $(SRCDIR)\logodownload.c -o "$(LOCOBJ)\logodownload.o"
$(LOCOBJ)\keysdownload.o: $(SRCDIR)\keysdownload.c
        $(GCC) $(SRCDIR)\keysdownload.c -o "$(LOCOBJ)\keysdownload.o"
$(LOCOBJ)\http_handler.o: $(SRCDIR)\http_handler.c
        $(GCC) $(SRCDIR)\http_handler.c -o "$(LOCOBJ)\http_handler.o"
		
clean:
	del /q $(LOCOBJ)\*.o
	del /q $(LOCOBJ)\$(NAME).elf
	del /q $(LOCOBJ)\$(NAME).dasm
	del /q $(LOCOBJ)\$(NAME).map
	del /q $(LOCOBJ)\$(NAME).bin
