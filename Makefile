OUTDIR = $(shell pwd)/bin

all: CodeAlchemist

CodeAlchemist:
	dotnet build -c Release -o $(OUTDIR)

clean:
	dotnet clean
	rm -rf $(OUTDIR)
	-rm -rf /codealc/bug/*
	-rm -rf /codealc/pre/*
	-rm -rf /codealc/gen/*
	-rm -rf /codealc/tmp/*
