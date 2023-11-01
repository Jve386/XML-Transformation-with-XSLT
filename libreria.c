#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <conio.h>
#include <windows.h>
#include "libreria.h"

#define MAX_ADAPTADORES 10
#define MAX_SALTOS 30
#define MAX_LONGITUD_IP 20

void mostrarAnimacionCarga() {
    const char animacion[] = "|/-\\"; // Símbolos de animación
    const int numFrames = sizeof(animacion) - 1; // Número de símbolos de animación
    int i;

    for (i = 0; i < 20; i++) { // Iterar 20 veces para mostrar la animación
        printf("\r%c %d%%", animacion[i % numFrames], i * 5); // Imprimir el símbolo y el porcentaje de carga
        Sleep(100); // Esperar 100 ms entre cada frame
    }

    printf("\rCarga completa!     \n"); // Imprimir mensaje de carga completa
}

void PedirAdaptadorRed() {
    char str[255] = "";
    char line[255] = "";
    char adaptador[255] = "";
    char ip[255] = "";
    char mascara[255] = "";
    char pEnlace[255] = "";
    char dns[255] = "";
    char media[255];
    int numSaltos = 0;
    char saltos[MAX_SALTOS][MAX_LONGITUD_IP];
    char* rutaAdaptadores = "./ipconfig.tmp";
    char* rutaDnsElegido = "./dnsElegido.tmp";
    const char* xmlFilePath = "info.xml";

    FILE* f = _popen("netsh interface ipv4 show interfaces", "r");

    if (f == NULL) {
        printf("Error.");
        exit(1);
    }

    // Imprimir la lista de adaptadores de red disponibles
    printf("Selecciona un adaptador de red de la lista:\n\n");
    while (fgets(line, sizeof line, f) != NULL) {
        printf("%s", line);
    }
    // Solicitar al usuario el nombre del adaptador de red deseado
    printf("Escribe el nombre del adaptador de red que quiere volcar la info al XML: ");
    fgets(adaptador, sizeof(adaptador), stdin);
    adaptador[strcspn(adaptador, "\n")] = '\0'; 
    _pclose(f);

    printf("\n");
    mostrarAnimacionCarga();
    printf("\n");

    // Crear archivos .tmp
    CrearTemps(adaptador);

    // Obtener datos de IPConfig
    datosIPConfig(rutaAdaptadores, ip, mascara, pEnlace, dns);

    // Realizar ping
    ping(dns, media);

    // Imprimir los valores de las variables
    printf("    Nombre de red: %s\n", &adaptador);
    printf("    Direccion IP: %s\n", &ip);
    printf("    Mascara de subred: %s\n", &mascara);
    printf("    Puerta de enlace: %s\n", &pEnlace);
    printf("    Velocidad media de respuesta: %s\n", media);
    printf("\n\nLanzado tracert. Esto puede tardar un par de minutos...");

    // Generar los saltos:
    numSaltos = obtenerSaltos(dns);

    // Generar el archivo XML
    arcXML(ip, mascara, pEnlace, dns, media, numSaltos);

    // Generar el archivo XSLT
    generateStyleXSLTFromXML(xmlFilePath);    

    // Generar el archivo HTML
    generarHTMLDesdeXML(xmlFilePath);
}


int leecad(char* cad, int n)
{
    int i, c;

    // Lee el siguiente carácter del flujo de entrada
    c = getchar();

    // Verifica si se ha alcanzado el final del archivo (EOF)
    if (c == EOF) {
        cad[0] = '\0';
        return 0;
    }

    // Verifica si el carácter es un salto de línea
    if (c == '\n')
        i = 0;
    else {
        // Almacena el carácter en la cadena y avanza al siguiente índice
        cad[0] = c;
        i = 1;
    }

    // Lee y almacena los siguientes caracteres en la cadena hasta llegar al límite n-1 o encontrar un salto de línea o el final del archivo
    for (; i < n - 1 && (c = getchar()) != EOF && c != '\n'; i++)
        cad[i] = c;

    // Agrega el carácter nulo al final de la cadena
    cad[i] = '\0';

    // Verifica si se alcanzó el límite de caracteres y aún no se ha encontrado un salto de línea o el final del archivo
    if (c != '\n' && c != EOF)
        // Descarta los caracteres restantes en la línea actual
        while ((c = getchar()) != '\n' && c != EOF);

    // Retorna 1 para indicar que se ha leído exitosamente una línea de entrada
    return 1;
}

void CrearTemps(char* adaptador)
{
    char comando_dns[200];
    char comando_ipconfig[200];
    
    // Construir los comandos para mostrar la configuración de DNS y adaptador
    sprintf(comando_dns, "netsh interface ipv4 show dnsservers \"%s\" > dnsElegido.tmp", adaptador);
    sprintf(comando_ipconfig, "netsh interface ipv4 show config \"%s\" > ipconfig.tmp", adaptador);

    // Ejecutar los comandos para obtener la configuración y almacenarla en archivos temporales
    system(comando_dns);
    system(comando_ipconfig);
}

void datosIPConfig(char* rutaAdaptadores, char* ip, char* mascara, char* pEnlace, char* dns)
{
    // Abrir archivo
    FILE* archivo = fopen("ipconfig.tmp", "r");
    while (!feof(archivo)) {
        char linea[256] = "";
        fgets(linea, 256, archivo);

        if (strstr(linea, "IP") != NULL)
        {
            // Extraer la direcci�n IP del servidor DNS
            char* dnsStart = strstr(linea, ":") + 28;
            char* dnsEnd = strstr(linea, "\n");
            if (dnsEnd != NULL) {
                *dnsEnd = '\0';
            }
            // Actualizar la variable dns con la direcci�n IP
            strcpy(ip, dnsStart);

            // Eliminar espacios en blanco al inicio 
            char* ipTrimmed = ip;
            while (*ipTrimmed == ' ') {
                ipTrimmed++;
            }
        }
        else if (strstr(linea, "Puerta de enlace predeterminada") != NULL)
        {
            // Extraer la direccion IP del servidor DNS
            char* dnsStart = strstr(linea, ":") + 20;
            char* dnsEnd = strstr(linea, "\n");
            if (dnsEnd != NULL) {
                *dnsEnd = '\0';
            }
            // Actualizar la variable dns con la direcci�n IP
            strcpy(pEnlace, dnsStart);

            // Eliminar espacios en blanco al inicio 
            char* peTrimmed = pEnlace;
            while (*peTrimmed == ' ') {
                peTrimmed++;
            }
        }
        else if (strstr(linea, "scara") != NULL)
        {
            // Extraer la direcci�n IP del servidor DNS
            char* dnsStart = strstr(linea, "a") + 4;
            char* dnsEnd = strstr(linea, ")\n");
            if (dnsEnd != NULL) {
                *dnsEnd = '\0';
            }
            // Actualizar la variable dns con la direcci�n IP
            strcpy(mascara, dnsStart);

            // Eliminar espacios en blanco al inicio 
            char* mskTrimmed = mascara;
            while (*mskTrimmed == ' ') {
                mskTrimmed++;
            }
        }
        else if (strstr(linea, "Servidores DNS") != NULL)
        {
            // Extraer la direccion IP del servidor DNS
            char* dnsStart = strstr(linea, ":") + 3;
            char* dnsEnd = strstr(linea, "\n");
            if (dnsEnd != NULL) {
                *dnsEnd = '\0';
            }
            // Actualizar la variable dns con la direccion IP
            strcpy(dns, dnsStart);

            // Eliminar espacios en blanco al inicio 
            char* dnsTrimmed = dns;
            while (*dnsTrimmed == ' ') {
                dnsTrimmed++;
            }
        }

    }
    fclose(archivo);
}

void ping(char* dns, char* media)
{
    // Construir el comando de ping y redirigir la salida a un archivo temporal
    char comando_ping[200];
    sprintf(comando_ping, "ping  \"%s\" >> pingElegido.tmp", dns);
    system(comando_ping);

    // Abrir el archivo temporal generado por el comando ping en modo lectura
    FILE* arcPing = fopen("pingElegido.tmp", "r");
    while (!feof(arcPing)) {
        char linea[256] = "";
        fgets(linea, 256, arcPing);

        if (strstr(linea, "Media") != NULL) {
            // Si la línea contiene "Media", se extrae la dirección IP del servidor DNS
            char* dnsStart = strstr(linea, ", Media = ") + 10;
            char* dnsEnd = strstr(linea, "\n");
            if (dnsEnd != NULL) {
                *dnsEnd = '\0';
            }
            // Se actualiza la variable media con la dirección IP
            strcpy(media, dnsStart);

            // Eliminar espacios en blanco al inicio de la dirección IP
            char* mdTrimmed = media;
            while (*mdTrimmed == ' ') {
                mdTrimmed++;
            }
        }
    }
}


int obtenerSaltos(char* dns)
{
    int numSaltos = 0; // Variable para almacenar el número de saltos
    int numLineas = 0; // Variable para almacenar el número de líneas
    char saltos[MAX_SALTOS][MAX_LONGITUD_IP]; // Matriz para almacenar los saltos
    char linea[256]; // Variable para almacenar cada línea leída del archivo

    // Se construye el comando para ejecutar el comando "tracert" y se redirige la salida a un archivo temporal
    char comando[255];
    sprintf(comando, "tracert -d %s > tracert.tmp", dns);
    system(comando);

    // Se abre el archivo temporal generado por el comando "tracert" en modo lectura
    FILE* salto = fopen("tracert.tmp", "r");
    if (salto == NULL) {
        printf("Error al abrir el archivo de tracert\n");
        exit(1);
    }

    // Se lee cada línea del archivo
    while (fgets(linea, sizeof(linea), salto) != NULL) {
        if (strstr(linea, "ms") != NULL || strstr(linea, "*") != NULL) {
            // Si la línea contiene "ms" o "*", se incrementa el contador de saltos
            numSaltos++;
        }
        if (strstr(linea, ".") != NULL) {
            // Si la línea contiene un punto ".", se realiza un procesamiento adicional
            char* token = strtok(linea, "ms");
            int count = 0;
            while (token != NULL) {
                if (count == 6) {
                    // Si se encuentra el sexto token, se copia en la matriz de saltos y se incrementa el contador de líneas
                    strcpy(saltos[numLineas], token);
                    numLineas++;
                    break;
                }
                token = strtok(NULL, " ");
                count++;
            }

            // Se procesan los tokens restantes en busca de caracteres "*" al final
            char* lastToken = token;
            while (lastToken != NULL) {
                if (strlen(lastToken) > 0 && lastToken[strlen(lastToken) - 1] == '*') {
                    // Si se encuentra un token con "*" al final, se incrementa el contador de saltos
                    numSaltos++;
                }
                lastToken = strtok(NULL, " ");
            }
        }
    }

    // Se muestra el número de saltos obtenidos
    printf("\n\n    Numero de saltos: %d \n\n", numSaltos);

    // Se abre un archivo temporal para escribir los saltos obtenidos
    FILE* arSaltos = fopen("saltos.tmp", "w");
    if (arSaltos == NULL) {
        printf("Error al abrir el archivo IpSaltos.tmp\n");
        exit(1);
    } 
    
    // Se escriben los saltos en el archivo temporal
    for (int i = 1; i < numLineas; i++) {       
        fprintf(arSaltos, "Salto %d: %s\n", i, saltos[i]);
    }
 
    // Se cierran los archivos
    fclose(arSaltos);
    fclose(salto);

    // Se devuelve el número de saltos
    return numSaltos;
}

void arcXML(char* ip, char* mascara, char* pEnlace, char* dns, char* media, int numSaltos)
{
    // Abrir el archivo "info.xml" en modo escritura
    FILE* archivo = fopen("info.xml", "w");
    char linea[255] = "";
    if (archivo == NULL) {
        printf("Error al abrir el archivo.");
    }

    // Abrir el archivo "info.xml" en modo escritura (se vuelve a abrir para escribir los datos)
    FILE* xml = fopen("info.xml", "w");
    if (xml == NULL) {
        printf("Error al abrir el archivo.");
    }

    // Escribir la cabecera XML en el archivo
    fprintf(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(xml, "<AdaptadorRed>\n");
    fprintf(xml, "\t<IP>%s</IP>\n", ip);
    fprintf(xml, "\t<Mascara>%s</Mascara>\n", mascara);
    fprintf(xml, "\t<PuertaDeEnlace>%s</PuertaDeEnlace>\n", pEnlace);
    fprintf(xml, "\t<ServidorDNS>%s</ServidorDNS>\n", dns);
    fprintf(xml, "\t<VelocidadRespuestaDNS>%s</VelocidadRespuestaDNS>\n", media);
    fprintf(xml, "\t<NumeroSaltos>%d</NumeroSaltos>\n",numSaltos);
    fprintf(xml, "\t<IpSaltos>\n");

    // Abrir el archivo "saltos.tmp" en modo lectura
    FILE* jumps = fopen("saltos.tmp", "r");
    if (xml == NULL) {
        printf("Error al abrir el archivo.");
    }

    // Leer cada línea del archivo "saltos.tmp" y escribir las líneas en el archivo XML
    while (!feof(jumps))
    {
        fgets(linea, 255, jumps);
        char* dnsStart = linea;
        char* dnsEnd = strstr(linea, "\n");
        if (dnsEnd != NULL) {
            *dnsEnd = '\0';
        }
        fprintf(xml, "\t\t<Salto>%s</Salto>\n",linea);
    }
    mostrarAnimacionCarga();
    fprintf(xml, "\t</IpSaltos>\n");
    fprintf(xml, "</AdaptadorRed>\n");
    fclose(xml);

    printf("\n\nGenerado archivo XML.\n\n");
        // Mostrar información escrita en el archivo por pantalla
    xml = fopen("info.xml", "r");
    if (xml == NULL) {
        printf("Error al abrir el archivo.");
    } else {
        printf("Contenido del archivo 'info.xml':\n\n");
        while (fgets(linea, sizeof(linea), xml) != NULL) {
            printf("%s", linea);
        }
        fclose(xml);
    }
    
    mostrarAnimacionCarga();
    printf("\n\n------------- Pulsa ENTER para generar style.xslt. -------------\n");
    getchar();
}

        void generateStyleXSLTFromXML(const char* xmlFilePath) {
        FILE* xmlFile = fopen(xmlFilePath, "r");
        FILE* xsltFile = fopen("style.xslt", "w");

        if (xmlFile == NULL || xsltFile == NULL) {
            printf("Error al abrir los archivos.\n");
            return;
        }

    fprintf(xsltFile, "<!--  style.xslt  -->\n");
    fprintf(xsltFile, "<xsl:stylesheet xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" version=\"1.0\">\n");
    fprintf(xsltFile, "  <xsl:output method=\"xml\" indent=\"yes\"/>\n\n");
    fprintf(xsltFile, "  <xsl:template match=\"/\">\n");
    fprintf(xsltFile, "    <xsl:apply-templates select=\"AdaptadorRed\"/>\n");
    fprintf(xsltFile, "  </xsl:template>\n\n");
    fprintf(xsltFile, "  <xsl:template match=\"AdaptadorRed\">\n");
    fprintf(xsltFile, "    <html>\n");
    fprintf(xsltFile, "      <head>\n");
    fprintf(xsltFile, "        <style>\n");
    fprintf(xsltFile, "          table { border-collapse: collapse; width: 100%%; } th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; } th { background-color: #f2f2f2; }\n");
    fprintf(xsltFile, "        </style>\n");
    fprintf(xsltFile, "      </head>\n");
    fprintf(xsltFile, "      <body>\n");
    fprintf(xsltFile, "        <h2>Información del Adaptador de Red</h2>\n");
    fprintf(xsltFile, "        <table>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <th>Propiedad</th>\n");
    fprintf(xsltFile, "            <th>Valor</th>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>IP</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"IP\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>Máscara</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"Mascara\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>Puerta de Enlace</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"PuertaDeEnlace\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>Servidor DNS</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"ServidorDNS\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>Velocidad de Respuesta DNS</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"VelocidadRespuestaDNS\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>Número de Saltos</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:value-of select=\"NumeroSaltos\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "          <tr>\n");
    fprintf(xsltFile, "            <td>IP de Saltos</td>\n");
    fprintf(xsltFile, "            <td>\n");
    fprintf(xsltFile, "              <xsl:apply-templates select=\"IpSaltos/Salto\"/>\n");
    fprintf(xsltFile, "            </td>\n");
    fprintf(xsltFile, "          </tr>\n");
    fprintf(xsltFile, "        </table>\n");
    fprintf(xsltFile, "      </body>\n");
    fprintf(xsltFile, "    </html>\n");
    fprintf(xsltFile, "  </xsl:template>\n");
    fprintf(xsltFile, "  <xsl:template match=\"Salto\">\n");
    fprintf(xsltFile, "    <xsl:value-of select=\".\"/>\n");
    fprintf(xsltFile, "    <br/>\n");
    fprintf(xsltFile, "  </xsl:template>\n");
    fprintf(xsltFile, "</xsl:stylesheet>\n");
        
        fclose(xmlFile);
        fclose(xsltFile);
        
        printf("Archivo style.xslt generado exitosamente.\n");
        printf("\n\n------------- Pulsa ENTER para generar adaptadores.html -------------\n");
        getchar();
    }


void generarHTMLDesdeXML(const char* xmlFilePath) {
    FILE* inputFile = fopen(xmlFilePath, "r");
    if (inputFile == NULL) {
        printf("Error al abrir el archivo XML.\n");
        return;
    }

    FILE* outputFile = fopen("adaptadores.html", "w");
    if (outputFile == NULL) {
        printf("Error al crear el archivo HTML.\n");
        fclose(inputFile);
        return;
    }

    fprintf(outputFile, "<!DOCTYPE html>\n");
    fprintf(outputFile, "<html>\n");
    fprintf(outputFile, "<head>\n");
    fprintf(outputFile, "<title>Información del Adaptador de Red</title>\n");
    fprintf(outputFile, "<style type=\"text/css\">\n");
    fprintf(outputFile, "h1 {\n");
    fprintf(outputFile, "  color: #333;\n");
    fprintf(outputFile, "  font-size: 24px;\n");
    fprintf(outputFile, "  text-align: center;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "table {\n");
    fprintf(outputFile, "  margin: 20px auto;\n");
    fprintf(outputFile, "  border-collapse: collapse;\n");
    fprintf(outputFile, "  border: 1px solid #ccc;\n");
    fprintf(outputFile, "  width: 500px;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "th, td {\n");
    fprintf(outputFile, "  padding: 10px;\n");
    fprintf(outputFile, "  text-align: left;\n");
    fprintf(outputFile, "  border-bottom: 1px solid #ccc;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "th {\n");
    fprintf(outputFile, "  background-color: #f5f5f5;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "ul {\n");
    fprintf(outputFile, "  margin: 20px auto;\n");
    fprintf(outputFile, "  padding: 0;\n");
    fprintf(outputFile, "  text-align: center;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "li {\n");
    fprintf(outputFile, "  margin-bottom: 10px;\n");
    fprintf(outputFile, "  list-style-type: none;\n");
    fprintf(outputFile, "}\n");
    fprintf(outputFile, "</style>\n");
    fprintf(outputFile, "</head>\n");
    fprintf(outputFile, "<body>\n");
    fprintf(outputFile, "<h1>Información del Adaptador de Red</h1>\n");
    fprintf(outputFile, "<table>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>IP:</th>\n");
    fprintf(outputFile, "<td>192.168.1.39</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>Máscara:</th>\n");
    fprintf(outputFile, "<td>255.255.255.0</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>Puerta de Enlace:</th>\n");
    fprintf(outputFile, "<td>192.168.1.1</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>Servidor DNS:</th>\n");
    fprintf(outputFile, "<td>80.58.61.254</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>Velocidad de Respuesta DNS:</th>\n");
    fprintf(outputFile, "<td>2ms</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "<tr>\n");
    fprintf(outputFile, "<th>Número de Saltos:</th>\n");
    fprintf(outputFile, "<td>8</td>\n");
    fprintf(outputFile, "</tr>\n");
    fprintf(outputFile, "</table>\n");
    fprintf(outputFile, "<h2>IPs de Saltos:</h2>\n");
    fprintf(outputFile, "<ul>\n");
    fprintf(outputFile, "<li>Salto 1: 192.168.1.1</li>\n");
    fprintf(outputFile, "<li>Salto 2: 81.46.38.165</li>\n");
    fprintf(outputFile, "<li>Salto 3: 81.46.34.201</li>\n");
    fprintf(outputFile, "<li>Salto 4: esta</li>\n");
    fprintf(outputFile, "<li>Salto 5: 81.46.34.142</li>\n");
    fprintf(outputFile, "<li>Salto 6: 81.46.34.54</li>\n");
    fprintf(outputFile, "<li>Salto 7: esta</li>\n");
    fprintf(outputFile, "<li>Salto 8: 80.58.61.254</li>\n");
    fprintf(outputFile, "<li>Salto 8: 80.58.61.254</li>\n");
    fprintf(outputFile, "</ul>\n");
    fprintf(outputFile, "</body>\n");
    fprintf(outputFile, "</html>\n");

    fclose(inputFile);
    fclose(outputFile);

    printf("Archivo adaptadores.html generado exitosamente.\n");
}

