// Programme Arduino Uno pour mesure le temps entre Appui sur switch (départ)
// et switch(arrivée) avec écran LCD Keypad Shield de Sainsmart
// La touche RIGHT du clavier remet le temps mémorisé au maxi 999999

// Objectif : mesurer le temps mis pour effectuer un parcours avec un robot.

/*
  NE PAS UTILISER CES PINS utilisées par l'écran LCD
  The circuit:
 * LCD RS pin to digital pin 8
 * LCD Enable pin to digital pin 9
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
 * LCD BL pin to digital pin 10
 * KEY pin to analogl pin 0
 */
// Bibliothèque pour gérer l'écran LCD
#include 
// Bibliothèque pour gérer l'EEPROM
#include 
// Adresse de la data dans l'EEPROM
int adresse = 0;

// Connexion de l'écran sur les pins de l'Arduino
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);


// Lecture des boutons poussoirs via une entrée analogique
// Niveaux correspondant aux touches
int adc_key_val[5] ={50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;
int flag = 0;
int fini = 1; // indique que le parcours est fini


// Quelques variables qui serviront (ou pas)
int start = 1;
int stop = 0;
unsigned long debounce ;
unsigned long tempo ;
unsigned long MS;
unsigned long MS1;
unsigned long MS2;
unsigned long meilleurTemps = 123456;
unsigned long MAXI = 9999999;
int valid = 0; // Validation de la remise au temps maxi
int jeu = 0; 

// Pour convertir les chiffres en chaine de caractère
char buf[16];


// Partie du programme qui se charge d'initialiser 
// Ce qui doit l'être
void setup()
{
  //Initialise la communication série
  Serial.begin(9600);
  lcd.clear();       // Efface l'écran
  lcd.begin(16, 2);  // Déclare une instance de l'écran


  pinMode(3, OUTPUT);  // Déclarer la LED indiquant que le jeu est prêt
  pinMode(12, OUTPUT);  // Déclarer la LED indiquant que l'animateur a autorisé le départ
  
  digitalWrite(3, HIGH); // Allumer la LED
  digitalWrite(12, HIGH); // Allumer la LED
    delay(1000);
  digitalWrite(3, LOW);  // Eteindre la LED pour montrer le démarrage du programme
  digitalWrite(12, LOW);  // Eteindre la LED pour montrer le démarrage du programme
 

  

  pinMode(A5, INPUT_PULLUP);  // Entrée D0 : START
  pinMode(11, INPUT_PULLUP);  // Entrée D1 : STOP
  pinMode(2, INPUT_PULLUP);  // Entrée D2 : Validation par l'animateur (valid quand est à LOW
  

  // Relire contenu de l'EEPROM et le ramener en mémoire
  // Meilleur temps
  EEPROM_readAnything(0, meilleurTemps);
  Serial.print("Meilleur temps: "); 
  Serial.println(meilleurTemps);           // Afficher le meilleur temps sur la console pour vérification

// Effacer deuxième ligne
lcd.setCursor(0,1); 
lcd.print("                "); 

}



// Boucle principale du programme
//======================================================================================
void loop()
{

// Positionner le curseur au début de la ligne 1
lcd.setCursor(0,0); 
lcd.print("A battre        "); 
// Positionner le curseur après le texte
lcd.setCursor(9,0);
//lcd.print("       "); 
// afficher le meilleur temps
lcd.print(meilleurTemps); 


// Allumer la LED PRET si les switch sont bien positionnés)
// c'est une simple indication pour que l'animateur valide le début du jeu 
// Switch de Départ appuyé, switch Arrivée relaché
if ((digitalRead(A5) == LOW) && (digitalRead(11) == HIGH))
{
//    Serial.println("Switches prêts");
    digitalWrite(3, HIGH); // Allumer la LED  PRET
}
else
{
    digitalWrite(3, LOW); // Eteindre la LED  PRET
}

// Validation du jeu par l'animateur
if (digitalRead(2) == LOW)
{
    digitalWrite(12, HIGH); // Allumer la LED  JEU
}
else
{
    digitalWrite(12, LOW); // Eteindre la LED  JEU
}

  
adc_key_in = analogRead(0);    // read the value from the sensor  
 
// Fonctionnement manuel avec les boutons du clavier
// SELECT  démarre le comptage
// LEFT    arrête le comptage
// RIGHT suivi de UP  remet le temps à 9999999 s

key = get_key(adc_key_in);    // convert into key press
  if (key != oldkey)  // if keypress is detected
  {
      //    delay(50);    // wait for debounce time Ne pas utiliser car arrete le programme
      debounce = millis();
      while ((millis() -debounce) <50) { ; } adc_key_in = analogRead(0); // read the value from the sensor key = get_key(adc_key_in); // convert into key press if (key != oldkey) { oldkey = key; Serial.println("Key détectée"); Serial.println(key); delay(1000); } } // On a récupéré la touche appuyée // Si c'est right on remet le temps au maxi // Remettre le Meilleur temps au maxi // On ne doit pas être en train de compter if (key == 0 && flag == 0) { Serial.println("Key à l'entrée de la boucle"); Serial.println(key); delay(1000); Serial.println("Reset du Maxi"); EEPROM_writeAnything(0, MAXI); lcd.setCursor(0,0); lcd.print("A battre "); // Positionner le curseur après le texte lcd.setCursor(9,0); // afficher le meilleur temps lcd.print(meilleurTemps); lcd.setCursor(0,1); lcd.print(" "); // Relire contenu de l'EEPROM et le ramener en mémoire // Temps MAXI EEPROM_readAnything(0, meilleurTemps); Serial.print("Temps MAxi : "); Serial.println(meilleurTemps); // Afficher le meilleur temps sur la console pour vérification valid = 0; // On remet valid à 0 } // Si le flag est à 0 on n'est pas en train de compter => on attend le départ
// =============================================================================
if (flag == 0)
{
      // Attendre que le robot soit en position sur le départ
      // Start appuyé - Stop relâché
      if ((digitalRead(A5) == LOW && digitalRead(11) == HIGH)) 
      {
            // Ecrire ATTENDEZ sur la deuxième ligne
            lcd.setCursor(0,1); 
            lcd.print(" ATTENDEZ VALID ");
       }
       else
       {
            // Ecrire POSITIOONEZ ROVER sur la deuxième ligne
            lcd.setCursor(0,1); 
            lcd.print("PLACEZ LE ROVER ");
       }

      // Attendre la validation
      // Start appuyé Stop relâché  Bouton validé OK
      if ((digitalRead(A5) == LOW  &&  digitalRead(11) == HIGH && digitalRead(2) == LOW) )
          {
            // Ecrire PARTEZ sur la deuxième ligne
            lcd.setCursor(0,1); 
            lcd.print("     PARTEZ     ");
            fini = 0;
          }

// Démarrage du jeu 
// Start relâché  Stop  relâché    Validé = 0K
//================================================================================
if ( digitalRead(A5) == HIGH && digitalRead(11) == HIGH && digitalRead(2) == LOW && fini == 0 ) 
    {
      // Déclencher le chrono
      MS1 = millis();
      Serial.println("Debut comptage MS1");
      Serial.println(MS1);
      flag = 1; // Indique qu'on compte
      // Temporisation antirebond
      while ((millis() - tempo) <100) {
        ;
        }
    }

// Fin du if (flag = 0)
}

// S'exécute si le flag est à 1 : Comptage en cours
// Start relâché  Stop  appuyé    Validé = 0K
// Arrêt du jeu
// ==================================================================================
if (flag == 1)
{
    if ( (digitalRead(A5) == HIGH && digitalRead (11) == LOW && digitalRead (2) == LOW) ) 
        { 
          MS2 = millis();
          // On a fini le parcours tant que le bouton valid est on on bloque
          fini = 1;
                    
          flag = 0; // Indique qu'on arrête le comptage
    
          MS = MS2 - MS1;
          Serial.println("MS a l'arret du jeu");
          Serial.println(MS);
          Serial.println("FLAG a l'arret du jeu");
          Serial.println(flag);
    
          ltoa(MS, buf, 10);
          lcd.setCursor(0, 1);  //Position 0, ligne 2
          lcd.print(MS);
          delay (5000);
          if (MS < meilleurTemps)
          {
             meilleurTemps = MS;
             lcd.setCursor(0, 1);  //Position 0, ligne 2
             lcd.print("NOUVEAU RECORD !");
             delay (5000);
             // Ecrire la nouvelle valeur en EEPROM
             EEPROM_writeAnything(0, MS); 
             delay(3000);
    
          }
        }
    else
        {
        // On est entrain de compter : flag à 1
        // Mais on n'arrête pas le comptage
        // Afficher le temps en cours
        lcd.setCursor(0, 1);  //Position 0, ligne 2
        lcd.print("                ");
        lcd.setCursor(0, 1);  //Position 0, ligne 2
        lcd.print(millis()-MS1);
        }
    
    
    // Si on compte et que le bouton validation n'est plus OK
    // Par exemple car triche ou abandon    
    if (digitalRead(2) == HIGH)
        { 
            Serial.println("Validation enlevée");
            // On arrête le comptage
            flag = 0;
        }
}

// Temporisation d'affichage
while ((millis() - tempo) <10) {
  ;
  }

}


// FONCTIONS
// ============================================================

// Convert ADC value to key number
int get_key(unsigned int input)
{
    int k;
   
    for (k = 0; k < NUM_KEYS; k++)
    {
      if (input < adc_key_val[k]) { return k; } } if (k >= NUM_KEYS)k = -1;  // No valid key pressed
    return k;

}


//We create two fucntions for writing and reading data from the EEPROM
template  int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
    return i;
}
template  int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}
