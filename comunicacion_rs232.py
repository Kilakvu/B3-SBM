import serial
import time

# --- CONFIGURACIÓN ---
# Ajusta el puerto COM al que esté conectada tu NUCLEO (ej: 'COM3' en Windows o '/dev/ttyACM0' en Linux)
PORT = 'COM3' 
BAUD = 9600

def calcular_trama(cmd, payload=""):
    """
    Construye la trama siguiendo el formato: SOH CMD LEN Payload EOT
    """
    SOH = 0x01
    EOT = 0xFE
    
    # Payload debe ser ASCII (bytes)
    payload_bytes = payload.encode('ascii')
    
    # LEN = SOH(1) + CMD(1) + LEN(1) + Payload(n) + EOT(1)
    length = 1 + 1 + 1 + len(payload_bytes) + 1
    
    trama = bytearray([SOH, cmd, length]) + payload_bytes + bytearray([EOT])
    return trama

def enviar_y_esperar(ser, nombre, trama):
    print(f"\n--- {nombre} ---")
    print(f"Enviando (HEX): {trama.hex(' ').upper()}")
    ser.write(trama)
    
    # Esperar un poco a que el STM32 procese y responda
    time.sleep(0.5)
    
    if ser.in_waiting > 0:
        respuesta = ser.read(ser.in_waiting)
        print(f"Respuesta (HEX): {respuesta.hex(' ').upper()}")
        # Intentar extraer el payload de la respuesta (si existe)
        if len(respuesta) > 4:
            payload_resp = respuesta[3:-1].decode('ascii', errors='ignore')
            print(f"Payload respuesta: '{payload_resp}'")
    else:
        print("No se recibió respuesta (¿Estás en modo PROGRAMACIÓN?)")

def main():
    try:
        # Abrir puerto serie
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Conectado a {PORT} a {BAUD} baudios.")

        while True:
            print("\n--- MENÚ DE COMANDOS ---")
            print("1. Puesta en hora (15:20:15)")
            print("2. Establecer Referencia (1000)")
            print("3. Solicitar todas las medidas")
            print("4. Borrar medidas")
            print("5. Salir")
            
            opcion = input("Selecciona una opción: ")

            if opcion == '1':
                hora = input("Introduce hora (HH:MM:SS) [Enter para 15:20:15]: ") or "15:20:15"
                trama = calcular_trama(0x20, hora)
                enviar_y_esperar(ser, "Puesta en Hora", trama)
            
            elif opcion == '2':
                ref = input("Introduce lux (ej: 1000): ") or "1000"
                trama = calcular_trama(0x25, ref)
                enviar_y_esperar(ser, "Set LREF", trama)
            
            elif opcion == '3':
                trama = calcular_trama(0x55)
                enviar_y_esperar(ser, "Leer Medidas", trama)
            
            elif opcion == '4':
                trama = calcular_trama(0x60)
                enviar_y_esperar(ser, "Borrar Memoria", trama)
            
            elif opcion == '5':
                break
            else:
                print("Opción no válida.")

        ser.close()
        print("Puerto cerrado.")

    except serial.SerialException as e:
        print(f"Error al abrir el puerto: {e}")
    except KeyboardInterrupt:
        print("\nPrograma terminado.")

if __name__ == "__main__":
    main()
