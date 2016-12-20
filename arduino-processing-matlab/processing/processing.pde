
import processing.serial.*;

Serial myPort;
String myRawString = "";


int[][] image = new int[30][30];
int row = 0;
int col = 0;

int driftX = 0;
int driftY = 0;

void setup() {
  size(300, 330, FX2D);
  pixelDensity(2);
  String portName = Serial.list()[1];    //Change this port depending on your system.
  myPort = new Serial(this, portName, 115200);
}

void draw() {
  
  fill(255, 0, 0);
  rect(0, 0, 300, 30);

  fill(255, 255, 0);
  textSize(15);
  text("Ling & Thomas", 10, 20); 
  

  while (myPort.available() > 0) {
    myRawString = myPort.readStringUntil('\n');
    if (myRawString != null) {
      
      if(myRawString.charAt(0) == 'S' | myRawString.charAt(0) == 'E'){
        row = 0;
        col = 0;
      }
      if(myRawString.charAt(0) == 'I'){
        String strX = myRawString.substring(1, 4);
        String strY = myRawString.substring(5, 8);
      
        driftX = parseInt(strX)*-1;
        driftY = parseInt(strY);
      
        //println("X: "+ driftX);
        //println("Y: "+ driftY);
        
        fill(255, 255, 0);
        textSize(15);
        text(driftX, 210, 20);
        text(driftY, 280, 20);
      }
      else{
        int[] line = int(split(myRawString, "-"));
        if(line.length > 30 && row < 30){
          for (int i = 0; i < 30; i = i+1) {
            image[i][row] = line[i];
            
            stroke(image[i][row]);
            fill(image[i][row]);
            rect(i*10, row*10+30, 10, 10);
          }
          
          row++;
        }
      }

    }
  }

}