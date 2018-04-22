
SRCS_uploader	= uploader.c hidapi/linux/hid.c
SRCS_encryptor	= encrypt.c ../speck.c
INCS			= hidapi/hidapi/hidapi.h
CCFLAGS			= -iquote hidapi/hidapi -iquote ..
LDFLAGS			= -ludev

all: uploader encryptor

uploader: $(SRCS_uploader) hidapi/hidapi/hidapi.h
	gcc -o $@ $(SRCS_uploader) $(CCFLAGS) $(LDFLAGS)

encryptor: $(SRCS_encryptor)
	gcc -o $@ $(SRCS_encryptor) $(CCFLAGS) $(LDFLAGS)

clean:
	rm -f uploader encryptor
 
