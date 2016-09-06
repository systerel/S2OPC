BUILD_DIR=./build
CERT_DIR=./cert
OUT=./out

.PHONY : all clean

default: all

all: 
	\mkdir -p $(BUILD_DIR) $(OUT)
	\mkdir -p $(OUT)/revoked $(OUT)/untrusted $(OUT)/trusted \
	$(OUT)/client_private $(OUT)/server_private \
	$(OUT)/client_public $(OUT)/server_public
	\cp $(CERT_DIR)/cacert.der $(OUT)/trusted
	\cp $(CERT_DIR)/client.key $(OUT)/client_private
	\cp $(CERT_DIR)/client.der $(OUT)/client_public
	\cp $(CERT_DIR)/server.key $(OUT)/server_private
	\cp $(CERT_DIR)/server.der $(OUT)/server_public
	\cp Makefile.template $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR)

clean:
	\rm -rf $(BUILD_DIR) $(OUT)
	\rm -rf *.der *.key
	\rm -rf bin
	\rm -rf lib/linux src/linux AnsiCSample/linux
	\rm -rf lib/win32 src/win32 AnsiCSample/win32

check: all
	$(MAKE) -C $(BUILD_DIR) check
