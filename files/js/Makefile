ALL_FILES := common.js confPage.js editPage.js listPage.js
ALL_FILES += style.css
UPLOAD = python ../../python/upload.py
URL= http://192.168.0.206/sd/static

upload: $(ALL_FILES)
	cp $(ALL_FILES) ../../sdcard/static
	$(UPLOAD) $(URL) $(ALL_FILES)
