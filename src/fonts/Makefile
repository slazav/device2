all: font_koi8_r.inc

%.inc: %.txt font_cnv
	./font_cnv $< > $@

clean:
	rm -f *.inc
