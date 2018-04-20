.SUFFIXES: .htm .bdy

TEMPLATE_FILES := config.htm edit.htm index.htm list.htm
UPLOAD = python ../../python/upload.py
URL= http://192.168.0.206/sd/TEMPLATE

.bdy.htm:
	cat preamble.htm $< postamble.htm > $@

$(TEMPLATE_FILES): preamble.htm postamble.htm

target: $(TEMPLATE_FILES)
	cp $(TEMPLATE_FILES) ../../sdcard/template/

upload: target
	$(UPLOAD) $(URL) $(TEMPLATE_FILES)
