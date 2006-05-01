
TARGET = Companion
RES_DEP = -DMAX_MIS=10 -DMARGIN=20
PRCNAME = GBKOS_Pal.prc

CODES = $(CODE1000).grc $(CODE1002).grc $(CODE2000).grc $(CODE1001).grc 

CODE1000 = code03e8
CODE1000_OBJS = entry.o DrawChars.o

CODE1002 = code03ea
CODE1002_OBJS = entry.o DrawChar.o

CODE1001 = code03e9
CODE1001_OBJS = entry.o AppSwitch.o Common.o

CODE2000 = code07d0
CODE2000_OBJS = entry.o $(TARGET)_panel.o Common.o

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

INCLUDE = -I/usr/local/palmdev/include -I/usr/local/palmdev/include/Core -I/usr/local/palmdev/include/Core/Hardware -I/usr/local/palmdev/include/Core/System -I/usr/local/palmdev/include/Core/UI -I/usr/local/palmdev/include/Core/System/Unix
EXT_INCLUDE = -I/usr/local/palmdev/include/Dynamic -I/usr/local/palmdev/include/Extensions/ExpansionMgr/ -I/usr/local/palmdev/include/Libraries -I/usr/local/palmdev/include/Libraries/ExgLocal -I/usr/local/palmdev/include/Libraries/INet -I/usr/local/palmdev/include/Libraries/Lz77 -I/usr/local/palmdev/include/Libraries/PalmOSGlue -I/usr/local/palmdev/include/Libraries/Pdi -I/usr/local/palmdev/include/Libraries/Sms -I/usr/local/palmdev/include/Libraries/Telephony -I/usr/local/palmdev/include/Libraries/Telephony/UI

ASMFLAGS = -Wall
CFLAGS = $(DEBUG_FLAG) $(OPT_FLAG) -palmos4 -Wall -O2 $(RES_DEP) # $(INCLUDE)
LDFLAGS = -Wall

PILRC = pilrc
OBJRES = m68k-palmos-obj-res
BUILDPRC = build-prc
PILOTXFER = pilot-xfer

all: $(PRCNAME)

.S.o:
	$(cc) $(ASMFLAGS) -c $<

.c.o:
	$(cc) $(CFLAGS) -c $<

.c.s:
	$(CC) $(CFLAGS) -S $<

.cc.o:
	$(cc) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<

.cc.s:
	$(CC) $(CFLAGS) -S $<

$(PRCNAME): $(TARGET).def $(CODES) $(TARGET).ro
	$(BUILDPRC)  -o $(PRCNAME) $(TARGET).def $(CODES) $(TARGET).ro


$(CODE1000).grc: $(CODE1000).out
	$(OBJRES) $< && \
	mv code0001.$<.grc $@ && \
	rm *.$<.grc

$(CODE1000).out: $(CODE1000_OBJS)
	$(LINK) $(LDFLAGS) -nostartfiles $(CODE1000_OBJS) -o $@

$(CODE1001).grc: $(CODE1001).out
	$(OBJRES) $< && \
	mv code0001.$<.grc $@ && \
	rm *.$<.grc

$(CODE1001).out: $(CODE1001_OBJS)
	$(LINK) $(LDFLAGS) -nostartfiles $(CODE1001_OBJS) -o $@

$(CODE1002).grc: $(CODE1002).out
	$(OBJRES) $< && \
	mv code0001.$<.grc $@ && \
	rm *.$<.grc

$(CODE1002).out: $(CODE1002_OBJS)
	$(LINK) $(LDFLAGS) -nostartfiles $(CODE1002_OBJS) -o $@

$(CODE2000).grc: $(CODE2000).out
	$(OBJRES) $< && \
	mv code0001.$<.grc $@ && \
	rm *.$<.grc

$(CODE2000).out: $(CODE2000_OBJS)
	$(LINK) $(LDFLAGS) -nostartfiles $(CODE2000_OBJS) -o $@

bin.res: $(TARGET).rcp $(TARGET)_res.hpp 
	-rm -f *.bin && \
	$(PILRC) $(TARGET).rcp . && \
	touch bin.res
	
$(TARGET).ro: $(TARGET).rcp $(TARGET)_res.hpp 
	-rm -f *.bin && \
	$(PILRC) -q -ro $(TARGET).rcp $(TARGET).ro 
	
send: $(PRCNAME)
	$(PILOTXFER) -i $(PRCNAME)

depend:
	makedepend -Y -I. *.c *.cpp *.cc

clean:
	-rm -f *.o *.out *.grc *.bin bin.res $(PRCNAME) *.ro *.bak *.BAK *~

veryclean: clean
	-rm -f $(PRCNAME) pilot.ram pilot.scratch

DrawChars.o: $(TARGET).hpp $(TARGET)_res.hpp
DrawChar.o: $(TARGET).hpp $(TARGET)_res.hpp
AppSwitch.o: $(TARGET).hpp $(TARGET)_res.hpp
