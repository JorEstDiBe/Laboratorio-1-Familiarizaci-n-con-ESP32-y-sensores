const int PIN_BOTON = 32;
const int PIN_RELE  = 22;

void setup() {

  Serial.begin(115200);

  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_RELE, OUTPUT);

  // relé apagado
  digitalWrite(PIN_RELE, HIGH);

  Serial.println("Sistema listo");
}

void loop() {

  int estadoBoton = digitalRead(PIN_BOTON);

  if (estadoBoton == LOW) {   // botón presionado
    digitalWrite(PIN_RELE, LOW); 
    Serial.println("RELE ACTIVADO");
  } 
  else {
    digitalWrite(PIN_RELE, HIGH);
  }

  delay(50);
}