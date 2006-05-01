all: build da lib
build:
	./buildNo.sh
	
ifdef GB2312	
TARGET = GBOS
CHARSET_FLAG = -D_GB2312_
else
TARGET = GBKOS
CHARSET_FLAG =
endif

DA_OBJS = MainForm.do 
COMMON_OBJS = common.o DrawChars.o CharsWidth.o CharsInWidth.o LineWidth.o CJKFont.o WordWrap.o \
			WidthToOffset.o TextMgr.o Compare.o IntlDispatch.o Encoding.o Big5.o\
			# FldHandleEvent.o 

APP_OBJS = MainForm.o GBKOS_Lib.ao AboutDialog.o ${COMMON_OBJS}
LIB_OBJS = GBKOS_Lib.o GBKOS_Lib-dispatch.o AboutDialog.o ${COMMON_OBJS}
COMMON_INC = common.h GBKOS_Lib.h PalmHiRes.h

ifdef DEBUG
DEBUG_FLAG = -g
OPT_FLAG = 
else 
DEBUG_FLAG = 
OPT_FLAG = -O2
endif


CC = m68k-palmos-g++
cc = m68k-palmos-gcc
LINK = m68k-palmos-gcc

ASMFLAGS = -Wall -g
CFLAGS = $(DEBUG_FLAG) $(OPT_FLAG) $(CHARSET_FLAG) -g -palmos4 -Wall 
LDFLAGS = -Wall -g

PILRC = pilrc
OBJRES = m68k-palmos-obj-res
BUILDPRC = build-prc


.SUFFIXES: .def .prc .rcp .ro .c .o .cc .o .cpp .o .s .o .S .o .cpp .do .cpp .do .cpp .ao

.S.o:
	$(cc) $(ASMFLAGS) -c $<

.s.o:
	$(cc) $(ASMFLAGS) -c $<

.c.o:
	$(cc) $(CFLAGS) -c $<

.c.do:
	$(cc) $(CFLAGS) -DTHIS_IS_DA -o $@ -c $<

.c.s:
	$(cc) $(CFLAGS) -S $<

.cc.o:
	$(CC) $(CFLAGS) -c $<

.cc.do:
	$(CC) $(CFLAGS) -DTHIS_IS_DA -o $@ -c $<

.cc.s:
	$(CC) $(CFLAGS) -S $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<

.cpp.do:
	$(CC) $(CFLAGS) -DTHIS_IS_DA -o $@ -c $<

.cpp.ao:
	$(CC) $(CFLAGS) -DTHIS_IS_APP -o $@ -c $<

.cpp.s:
	$(CC) $(CFLAGS) -S $<

.def.prc:
	$(BUILDPRC) -o $@ $^

.rcp.ro:
	$(PILRC) $(CHARSET_FLAG) -q -ro -o $@ $<
	


app: $(TARGET).prc
	
da: $(TARGET)_DA.prc
	
lib: $(TARGET)_Lib.prc

code0001.$(TARGET).grc: $(APP_OBJS) 
	$(CC) $(LDFLAGS) -o $(TARGET)  $^
	$(OBJRES) $(TARGET)

$(TARGET).prc: $(TARGET).def code0001.$(TARGET).grc $(TARGET).ro AboutDialog.ro Version.ro

$(TARGET)_DA.prc: code03e8.$(TARGET)_DA.grc $(TARGET)_DA.ro Version.ro
	$(BUILDPRC) -t "DAcc"  -n "GBKOS_DA" -c "dGBK" --no-check -o $@ $^
	
$(TARGET)_DA.ro: $(TARGET).rcp MainForm_res.h
	$(PILRC) -q -D THIS_IS_DA  -ro $(TARGET).rcp $(TARGET)_DA.ro 

code03e8.$(TARGET)_DA.grc: $(DA_OBJS)
	$(CC) $(LDFLAGS) -nostartfiles -e DA_Main -o $(TARGET)_DA  $^
	$(OBJRES) $(TARGET)_DA
	
$(TARGET)_Lib.prc: $(TARGET)_Lib.def $(TARGET)_Lib AboutDialog.ro
	$(BUILDPRC) -o $@ $^

$(TARGET)_Lib: ${LIB_OBJS}
	$(CC) -nostartfiles -o $@ $^


$(TARGET).ro: $(TARGET).rcp MainForm_res.h
	$(PILRC) -q -ro $(TARGET).rcp $(TARGET).ro 
	

clean:
	-rm -f   *.o *.ao *.do *.prc *.ro *.p *.bak *.BAK *~ *.bin *.grc ${TARGET} ${TARGET}_DA ${TARGET}_Lib GBKOS_Lib-dispatch.s

veryclean: clean
	-rm -f  pilot.ram pilot.scratch

MainForm.o: ${COMMON_INC} MainForm.h MainForm_res.h
common.o: ${COMMON_INC}
DrawChars.o: ${COMMON_INC}
CharsWidth.o: ${COMMON_INC}
CharsInWidth.o: ${COMMON_INC}
LineWidth.o: ${COMMON_INC}
CJKFont.o : ${COMMON_INC} CJKFont.h
WordWrap.o : ${COMMON_INC}
WidthToOffset.o : ${COMMON_INC}
TextMgr.o : ${COMMON_INC}
FldHandleEvent.o: ${COMMON_INC}
AboutDialog.ro: AboutDialog.rcp AboutDialog_Res.h
AboutDialog.o: AboutDialog.h AboutDialog.cpp AboutDialog_Res.h
Compare.o: ${COMMON_INC}
GBKOS_Lib.o: ${COMMON_INC} GBKOS_Lib.cpp AboutDialog.h
GBKOS_Lib-dispatch.o: GBKOS_Lib-dispatch.s
GBKOS_Lib-dispatch.s: GBKOS_Lib.def
	m68k-palmos-stubgen $<


MainForm.do: ${COMMON_INC} MainForm.h MainForm_res.h
common.do: ${COMMON_INC}
