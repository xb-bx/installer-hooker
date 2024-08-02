INCLUDES=\
		 -I ./libs/cJSON/ \
		 -I ./libs/curl-8.7.1/include/ \
		 -I ./libs/cwalk/include \
		 -I ./libs/utf8-utf16-converter/converter/include


MSVC_X32 ?= /opt/msvc/bin/x86/
MSVC_X64 ?= /opt/msvc/bin/x64/

ARCH ?= 64

CURL_VERSION = 8.7.1
CURL_SRC = ./libs/curl-$(CURL_VERSION)
CURL_BUILD = $(CURL_SRC)/build$(ARCH)
CURL_LIB = obj/$(ARCH)/libcurl-d.lib

CJSON_SRC = ./libs/cJSON
CJSON_BUILD = $(CJSON_SRC)/build$(ARCH)
CJSON_LIB = obj/$(ARCH)/libcjson.lib

CWALK_SRC = ./libs/cwalk
CWALK_BUILD = $(CWALK_SRC)/build$(ARCH)
CWALK_LIB = obj/$(ARCH)/cwalk.lib

UTFCONV_SRC = ./libs/utf8-utf16-converter
UTFCONV_BUILD = $(UTFCONV_SRC)/build$(ARCH)
UTFCONV_LIB = obj/$(ARCH)/converter.lib

LIBS =\
	  $(UTFCONV_LIB) \
	  $(CURL_LIB) \
	  $(CWALK_LIB) \
	  $(CJSON_LIB) 

DEFAULTLIBS="/DEFAULTLIB:ws2_32.lib /DEFAULTLIB:Crypt32.lib /DEFAULTLIB:Wldap32.lib /DEFAULTLIB:Normaliz.lib /DEFAULTLIB:MSVCRT /DEFAULTLIB:LIBCMT /DEFAULTLIB:Advapi32.lib /DEFAULTLIB:User32.lib /DEFAULTLIB:Shlwapi.lib"



ifeq ($(ARCH), 64)
	CMAKE = msvc-x64-cmake
else 
	CMAKE = msvc-x86-cmake
endif

$(CJSON_SRC):
	mkdir -p libs
	git clone https://github.com/DaveGamble/cJSON $(CJSON_SRC)

$(CJSON_BUILD): $(CJSON_SRC)
	$(CMAKE) -S $(CJSON_SRC) -B $(CJSON_BUILD) -DENABLE_CJSON_TEST=OFF -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" -DBUILD_SHARED_AND_STATIC_LIBS=On

$(CJSON_LIB): $(CJSON_BUILD)
	mkdir -p obj/$(ARCH)
	$(CMAKE) --build $(CJSON_BUILD) -j $(shell nproc)
	cp $(CJSON_BUILD)/libcjson.lib $(CJSON_LIB)

$(CWALK_SRC):
	mkdir -p libs
	git clone https://github.com/likle/cwalk $(CWALK_SRC)

$(CWALK_BUILD): $(CWALK_SRC)
	$(CMAKE) -S $(CWALK_SRC) -B $(CWALK_BUILD) -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" 

$(CWALK_LIB): $(CWALK_BUILD)
	mkdir -p obj/$(ARCH)
	$(CMAKE) --build $(CWALK_BUILD) -j $(shell nproc)
	cp $(CWALK_BUILD)/cwalk.lib $(CWALK_LIB)

$(UTFCONV_SRC):
	mkdir -p libs
	git clone https://github.com/Davipb/utf8-utf16-converter $(UTFCONV_SRC)

$(UTFCONV_BUILD): $(UTFCONV_SRC)
	$(CMAKE) -S $(UTFCONV_SRC) -B $(UTFCONV_BUILD) -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1" 

$(UTFCONV_LIB): $(UTFCONV_BUILD)
	mkdir -p obj/$(ARCH)
	$(CMAKE) --build $(UTFCONV_BUILD) -j $(shell nproc)
	cp $(UTFCONV_BUILD)/converter/converter.lib $(UTFCONV_LIB)

$(CURL_SRC): 
	mkdir -p libs
	wget https://curl.se/download/curl-$(CURL_VERSION).zip -O curl.zip
	unzip curl.zip -d libs
	rm curl.zip

$(CURL_BUILD): $(CURL_SRC)
	$(CMAKE) -S $(CURL_SRC) -B $(CURL_BUILD) -DBUILD_STATIC_LIBS=ON -DCMAKE_C_FLAGS_DEBUG="/MDd /Ob0 /Od /RTC1"

$(CURL_LIB): $(CURL_BUILD)
	mkdir -p obj/$(ARCH)
	$(CMAKE) --build $(CURL_BUILD) -j $(shell nproc)
	cp $(CURL_BUILD)/lib/libcurl-d.lib obj/$(ARCH)/libcurl-d.lib

obj/$(ARCH)/%.obj: src/%.c $(LIBS)
	cl /DWINE /c $< $(INCLUDES) /Fo"$@"
output/bootstrap.exe: obj/$(ARCH)/bootstrap.obj
	mkdir -p output
	link /NOIMPLIB /NOEXP obj/$(ARCH)/bootstrap.obj $(LIBS) "$(DEFAULTLIBS)" /OUT:output/bootstrap.exe

output/hook$(ARCH).dll: obj/$(ARCH)/hook.obj
	mkdir -p output 
	link /NOIMPLIB /NOEXP $< $(LIBS) "$(DEFAULTLIBS)" /OUT:$@ /DLL
output/hooker$(ARCH).exe: obj/$(ARCH)/hooker.obj
	mkdir -p output
	link /NOIMPLIB /NOEXP $< /OUT:$@
output/bootstrap.exe.manifest: src/manifest.xml
	cp $< $@
output/hooker$(ARCH).exe.manifest: src/manifest.xml
	cp $< $@

all: output/bootstrap.exe output/hooker$(ARCH).exe output/hook$(ARCH).dll output/bootstrap.exe.manifest output/hooker$(ARCH).exe.manifest 
.PHONY: all-arch
all-arch: 
	ARCH=32 PATH=$(MSVC_X32):$(PATH) make all
	ARCH=64 PATH=$(MSVC_X64):$(PATH) make all

.PHONY: clean
	rm -rf obj output 
.PHONY: distclean
distclean: clean
	rm -rf libs
