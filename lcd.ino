void displayBattery() {
  tft.setCursor(200, 10);  
  tft.setTextColor(WROVER_WHITE);  tft.setTextSize(2);  
  tft.println("B : " + String(voltageBattery));
}

void displayString(int row, int col, int textSize, String msg) {
  tft.setCursor(row, col);
  tft.setTextSize(textSize);  
  tft.println(msg);
}

bool isDisplayedMessage(String msg) {
  return strDisplay.equals(msg);
}

