# Compiler to Use    
CC = gcc                                                                                                                
                                                                                                                        
# Compile Flags                                        
CFLAGS = -ldl -fPIC -shared -D_GNU_SOURCE    
                                                                   
# Directories                                                  
OUTDIR = bin                                      
                                                  
TARGET = smoochum    
TARGETSO = libc.man.so.6»                                                                                              
                                                   
all: $(TARGET)                                                                                                         
                                                                         
$(TARGET): $(TARGET).c    
»---$(shell mkdir -p $(OUTDIR);)                                                              
»---$(CC) $(CFLAGS) -o $(OUTDIR)/$(TARGETSO) $(TARGET).c                                         
                                                                                                 
clean:                                                                                                                  
»---$(RM) -r $(OUTDIR) 
