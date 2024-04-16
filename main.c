//*****************************************************************************
//Universidad del Valle de Guatemala
//Programación de Microcontroladores
//Archivo:Lab_04
//Hardware:ATMEGA328P
//Autor:Adriana Marcela Gonzalez
//Carnet:22438
//*****************************************************************************
//Encabezado
//*****************************************************************************
#define F_CPU 16000000  // Define la frecuencia del CPU a 16 MHz

// Incluye las definiciones de los registros de entrada/salida
#include <avr/io.h> 
#include <util/delay.h> // Incluye la librería para las funciones de retraso
//#include <avr/interrupt.h>

//*****************************************************************************
//Definición de funciones y variables
//*****************************************************************************

void initADC(void);
void aumentar(void);   // Declaración de la función para aumentar 
void decrementar(void); // Declaración de la función para decrementar
void LaLuz(void); // Declaración de la función para actualizar los LEDs
void setup(void);      

unsigned char lic = 0; // Variable que guarda el valor del contador
unsigned char ADCPot = 0; //Varible para guardar el valor del potenciometro

//Tabla con los valores del 7 segmentos 
const uint8_t tabla_segmentos[16] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F, // 9
	0x77, // A
	0x7C, // B
	0x39, // C
	0x5E, // D
	0x79, // E
	0x71  // F
	
}

;

//*****************************************************************************
//ADC CODE
//*****************************************************************************

//Función para el ADC
void initADC(void){
	ADMUX = 0;
	
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	
	ADMUX |= (1<<ADLAR);
	
	ADCSRA = 0;
	
	ADCSRA |= (1<<ADEN);
	
	/*// Habilitamos ISR ADC
	ADCSRA |= (1<<ADIE);*/
	
	//Prescaler de 128 > 16M / 128 = 125kMz
	ADCSRA |= (ADPS2)|(ADPS1)|(ADPS0);
	
	DIDR0 |= (1<<ADC0D);
	
}
 //Función para convertir la señal analogica 
uint16_t valorADC (uint8_t admux_adc) {
	
	// Limpiar los bits del canal
	ADMUX &= 0xF0;
	
	// Seleccionar el canal
	ADMUX |= admux_adc;
	
	//Iniciando secuencia del ADC
	ADCSRA |= (1 << ADSC);
	
	while (ADCSRA & (1 << ADSC));
	
	// Devolver el valor del ADC
	return ADCH;
}
//*****************************************************************************
//Main
//*****************************************************************************

int main(void) {   // Función principal
	setup(); //Llama a la función setup
	//cli();
	initADC();//Llama a la función del adc 
	//sei();
	
	while (1) {        //LOOP
		
			PORTB |= (1 << PORTB2);
			LaLuz();    // Actualiza el estado de los LEDs
			
		// Verifica si el botón de incremento está presionado
		if (!(PINC & (1 << PORTC5))) {
			aumentar(); // Si lo está, llama a la función para aumentar el contador
			_delay_ms(500); // Espera un breve periodo de tiempo para evitar rebotes
			} else {
			_delay_ms(1); // Tiempo de espera
		}
		// Verifica si el botón de decremento está presionado
		if (!(PINC & (1 << PORTC1))) { 
			decrementar();// Si lo está, llama a la función para decrementar el contador
			_delay_ms(500);// Espera un breve periodo de tiempo para evitar rebotes
			} else {
			_delay_ms(1);
		}
		
		PORTB &= ~(1 << PORTB2);
		
		//Inicio del segmento del potenciometro 
		uint16_t ADCPot = valorADC(6); //Pasa el valor del pinc6 a valores digitales
		PORTC |= (1 << PORTC3);
		//Muestra en el primer segmento una parte del valor
		PORTD = tabla_segmentos[(ADCPot & 0xF0) >> 4];
		_delay_ms(4); 
		
		PORTC &= ~(1 << PORTC3);
		PORTD = 0x00;
		_delay_ms(4);
		
		PORTC |= (1 << PORTC4);
		//Muestra en el segundo segmento la parte que falta del valor 
		PORTD = tabla_segmentos[(ADCPot) & 0x0F];
		_delay_ms(4);
		
		PORTC &= ~(1 << PORTC4);
		PORTD = 0x00;
		_delay_ms(4);
		
		
		//Compara el potenciometro con el contador binario 
		if (ADCPot > lic){
			//Enciende un led si el valor del potenciometro es mayor 
			PORTB |= (1 << PORTB1);
		}
		else {
			PORTB &= ~(1 << PORTB1);
		}
		
		
		}
		
	return 0; // Retorno estándar de finalización
}
		
//*****************************************************************************
//Configuraciones
//*****************************************************************************

void setup(void) {
	DDRD = 0xFF; // Configura todos los pines del puerto D como salida
	PORTD = 0; // Inicializa todos los pines del puerto D en bajo (LEDs apagados)
	UCSR0B = 0; // Configura los registros de control de la UART en 0
	
	// Configura los pines PC5 y PC1 como entrada (Botones)
	DDRC &= ~(1 << DDC1) & ~(1 << DDC5);
	// Activa las resistencias pull-up internas para los pines PC5 y PC1
	PORTC |= (1 << PORTC1) | (1 << PORTC5);
	
	// Configura los pines PC3 y PC4 como salidas (transistores 7s)
	DDRC |= (1 << DDC3) & (1 << DDC4);
	
	// Configura el pin PC6 como entrada (potenciometro)
	DDRC &= ~(1 << DDC6);
	// Activa las resistencias pull-up internas 
	PORTC |= (1 << PORTC6);

	// Configura el pin PB2 como salida para la alarma
	DDRB |= (1 << DDB2);
	
	// Configura el pin PB1 como salida (transistor leds)
	DDRB |= (1 << DDB1);
	
}

//*****************************************************************************
//Subrutinas
//*****************************************************************************

/*ISR(ADC_vect) {
	if (ADCPot >= lic){ 
		PORTB |= (1 << PORTB1);   
		} 
		else {
		PORTB &= ~(1 << PORTB1);
}*/

// Implementación de la función para aumentar el contador
void aumentar() {       
	lic++;              // Incrementa la variable del contador
	if (lic > 255) {    // Verifica si el contador excede 255
		lic = 0;        // Reinicia el contador si excede 255
	}
}

// Implementación de la función para decrementar el contador
void decrementar() {    
	if (lic == 0) {     // Verifica si el contador está en su valor mínimo (0)
		lic = 255;      // Establece el contador en su valor máximo si está en 0
		} else {
		lic--;          // Decrementa la variable del contador si no está en 0
	}
}

// Implementación de la función para actualizar el estado de los LEDs
void LaLuz(void) {         
	for (int i = 0; i <= 7; i++) {  // Recorre cada bit del contador (8 bits)
		if (lic & (1 << i)) {    // Verifica si el bit correspondiente está encendido
			PORTD |= (1 << i);       // Enciende el LED correspondiente al bit
			} else {
			PORTD &= ~(1 << i);      // Apaga el LED correspondiente al bit
		}
	}
}
