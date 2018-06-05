void audioGenerateMp3(const char *str) {
  flagGenerator = true;  
  file = new AudioFileSourceSPIFFS(str);
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();

  if(mp3->begin(id3, out)) {
    Serial.println();
    Serial.println("MP3 Begin");
  }
  else {
    Serial.println("Begin Failed");
  }  
}

bool isGenerateMp3() {
  return flagGenerator;
}

void stop() {
  mp3->stop();
  
  delete(mp3);
  delete(id3);
  delete(out);    
  delete(file);  
  
//  mp3 = NULL;
  id3 = NULL;
  out = NULL;
  file = NULL;
  flagGenerator = false; 
}

