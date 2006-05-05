TARGET = PIME
APPNAME = "Palm IME"
APPID = "PIme"
DBTYPE = "DAcc"

OUTPUTDIR = output/
OBJS = $(OUTPUTDIR)PIME.o $(OUTPUTDIR)PIMEEngine.o

CXX = m68k-palmos-g++

CXXFLAGS = -Wall -O2

PILRC = pilrc
OBJRES = m68k-palmos-obj-res
BUILDPRC = build-prc

all: $(OUTPUTDIR)$(TARGET).prc

$(OUTPUTDIR)PIME.o: PIME.cpp ../Engine/PIMEEngine.h ../Engine/PIME_res.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUTDIR)PIMEEngine.o: ../Engine/PIMEEngine.cpp ../Engine/PIMEEngine.h ../Engine/PIME_res.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUTDIR)$(TARGET).prc: $(OUTPUTDIR)code03e8.grc $(OUTPUTDIR)bin.res
	$(BUILDPRC) --no-check -t $(DBTYPE) $@ $(APPNAME) $(APPID) $(OUTPUTDIR)code03e8.grc $(OUTPUTDIR)*.bin

$(OUTPUTDIR)code03e8.grc: $(OUTPUTDIR)$(TARGET)
	$(OBJRES) $(OUTPUTDIR)$(TARGET)
	mv code0001.$(TARGET).grc $@
	rm *.$(TARGET).grc

$(OUTPUTDIR)bin.res: ../Engine/PIME.rcp ../Engine/PIME_res.h
	rm -f $(OUTPUTDIR)*.bin
	$(PILRC) -I ../Engine/. -q ../Engine/PIME.rcp $(OUTPUTDIR)
	touch $(OUTPUTDIR)bin.res

$(OUTPUTDIR)$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -nostartfiles $(OBJS) -o $(OUTPUTDIR)$(TARGET) $(LIBS)

.PHONY: clean veryclean

clean:
	rm -f $(OUTPUTDIR)*.o $(OUTPUTDIR)*.bin $(OUTPUTDIR)bin.res $(OUTPUTDIR)*.grc $(OUTPUTDIR)$(TARGET) *~

veryclean: clean
	rm -f $(OUTPUTDIR)$(TARGET).prc
