#ifndef LIBRERIA_H_INCLUDED
#define LIBRERIA_H_INCLUDED

// #pragma once

// Declaración de la función para pedir el adaptador de red
void PedirAdaptadorRed();

// Declaración de la función para leer una cadena limitada por n caracteres desde la entrada
int leecad(char* cad, int n);

// Declaración de la función para crear archivos temporales utilizando el adaptador de red especificado
void CrearTemps(char* adaptador);

// Declaración de la función para obtener los datos de configuración de IP utilizando una ruta de adaptadores
void datosIPConfig(char* rutaAdaptadores, char* ip, char* mascara, char* pEnlace, char* dns);

// Declaración de la función para realizar un ping a una dirección IP utilizando una interfaz de red específica
void ping(char* dns, char* media);

// Declaración de la función para obtener el número de saltos para llegar a una dirección IP
int obtenerSaltos(char* dns);

// Declaración de la función para crear un archivo XML con los datos de IP, máscara, puerta de enlace, DNS, media y número de saltos
void arcXML(char* ip, char* mascara, char* pEnlace, char* dns, char* media, int numSaltos);

// Declaración de la función para crear un archivo XSLT a partir del XML generado
void generateStyleXSLTFromXML(const char* xmlFilePath);

// Declaración de la función para generar un archivo HTML a partir del XML generado
void generarHTMLDesdeXML(const char* xmlFilePath);

#endif

// LIBRERIA_H_INCLUDED