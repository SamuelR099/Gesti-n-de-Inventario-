/**
·*·@file·main·cpp
·*·@author·Samuel·Ramirez·
·*·@brief·Inventoy_Management·
·*·@version·1.0·
·*·@date·2024-07-01.
**
**
*/
#include <iostream>
#include <mysql.h>
#include <string>
#include <ctime>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <filesystem>
#include <sstream>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

//-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	v

//crea el nombre de la foto con su fecha  y hora 
std::string generateFileName() {
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo;
    localtime_s(&timeInfo, &in_time_t);
    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y%m%d_%H%M%S");
    return ss.str();
}

//-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	//-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	

// Función para escapar las barras invertidas
std::string escapeBackslashes(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\\') {
            result += "\\\\";
        }
        else {
            result += c;
        }
    }
    return result;
}

//-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	

// Funcion para escanear el Codigo QR

std::string scanQRCode() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        system("cls");
        std::cout << "No se pudo abrir la cámara" << std::endl;
        return "";
    }

    cv::QRCodeDetector qrDecoder;
    cv::Mat frame, gray;
    std::string data;

    std::cout << "Escaneando código QR. Presione 'q' para salir." << std::endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "Frame vacío" << std::endl;
            break;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        data = qrDecoder.detectAndDecode(gray);

        if (!data.empty()) {
            std::cout << "Código QR detectado: " << data << std::endl;
            break;
        }

        cv::imshow("QR Code Scanner", frame);
        if (cv::waitKey(1) == 'q') // Presiona 'q' para salir
            break;
    }

    cap.release();
    cv::destroyAllWindows();
    return data;
}

//-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	

int main() {
    MYSQL* conn;
    MYSQL_ROW row;
    MYSQL_RES* res;
    int qstate;

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
    
    // Obtener la fecha y hora actuales
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm timeInfo;
    localtime_s(&timeInfo, &in_time_t);

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
    
    // Establecer la conexión a la base de datos MySQL
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "ComputadorServer20", "c", 3306, NULL, 0)) {
        cout << "Error de conexion a la base de datos: " << mysql_error(conn) << endl;
        return 1;
    }

    string codigoQR = scanQRCode();
    if (codigoQR.empty()) {
        cout << "No se pudo escanear el código QR. Saliendo del programa." << endl;
        return 1;
    }

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
    
 // Consulta para obtener el objeto, la cantidad original y el id del código QR
    string consulta = "SELECT Object, Quantity_Original, id FROM items WHERE Code_QR = '" + codigoQR + "'";
    qstate = mysql_query(conn, consulta.c_str());
    if (!qstate) {
        res = mysql_store_result(conn);
        row = mysql_fetch_row(res);
        if (row) {
            string objeto = row[0];
            int Quantity_Original = stoi(row[1]);
            int itemId = stoi(row[2]); // Obtener el valor del id
            cout << "Objeto: " << objeto << endl;
            cout << "Cantidad original: " << Quantity_Original << endl;

            int NewQuantity;
            cout << "Ingrese la nueva cantidad: ";
            cin >> NewQuantity;

            // Formatear la fecha y la hora
            std::stringstream dateStream, timeStream;
            dateStream << std::put_time(&timeInfo, "%d/%m/%Y");
            timeStream << std::put_time(&timeInfo, "%H:%M:%S");
            string fecha = dateStream.str();
            string hora = timeStream.str();

            // Actualizar la cantidad, la fecha y la hora en la tabla "items"
            consulta = "UPDATE items SET Quantity = " + to_string(NewQuantity) + ", Fecha = '" + fecha + "', Hora = '" + hora + "' WHERE id = " + to_string(itemId);
            qstate = mysql_query(conn, consulta.c_str());
            if (!qstate) {
                cout << "La cantidad, la fecha y la hora se han actualizado correctamente." << endl;
            }
            else {
                cout << "Error al actualizar la cantidad, la fecha y la hora: " << mysql_error(conn) << endl;
            }

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
  
            string NewObservation;
            cout << "Ingrese una Observacion: ";
            cin.ignore(); // Ignorar el salto de línea pendiente
            getline(cin, NewObservation); // Leer una línea completa

            // Actualizar la observación en la tabla "observation"
            consulta = "UPDATE observation SET Observation = '" + NewObservation + "' WHERE id = " + to_string(itemId);
            qstate = mysql_query(conn, consulta.c_str());
            if (!qstate) {
                cout << "La observacion se ha actualizado correctamente." << endl;
            }
            else {
                cout << "Error al actualizar la observacion: " << mysql_error(conn) << endl;
            }

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
          
            // Código para capturar la foto
            cv::VideoCapture cap(0);
            if (!cap.isOpened()) {
                std::cout << "Error al abrir la cámara web" << std::endl;
                return -1;
            }
            cv::namedWindow("Webcam", cv::WINDOW_AUTOSIZE);
            std::string folderPath = "C:\\\\Users\\\\LENOVO\\\\Desktop\\\\FOTO\\\\"; // Cambia esto a la ruta de tu carpeta

            cout << "Presiona 's' para tomar una foto o 'q' para salir." << endl;

            while (true) {
                cv::Mat frame;
                cap >> frame;
                if (frame.empty()) {
                    std::cout << "Error al capturar el cuadro" << std::endl;
                    break;
                }
                cv::imshow("Webcam", frame);
                int key = cv::waitKey(30);
                if (key == 'q') {
                    break;
                }
                else if (key == 's') {
                    std::string fileName = folderPath + generateFileName() + ".jpg";
                    cv::imwrite(fileName, frame);
                    std::cout << "Imagen guardada como: " << fileName << std::endl;

                    // Actualizar la base de datos con la ruta de la foto
                    string consultaFoto = "UPDATE photo SET ruta_photo  = '" + fileName + "' WHERE id = " + to_string(itemId);
                    qstate = mysql_query(conn, consultaFoto.c_str());
                    if (!qstate) {
                        cout << "La ruta de la foto se ha guardado correctamente en la base de datos." << endl;
                    }
                    else {
                        cout << "Error al guardar la ruta de la foto en la base de datos: " << mysql_error(conn) << endl;
                    }

                    break; // Salir del bucle después de tomar la foto
                }
            }
            cap.release();
            cv::destroyAllWindows();
        }
        else {
            cout << "No se encontro el producto con el codigo QR especificado." << endl;
        }
        mysql_free_result(res);
    }
    else {
        cout << "Error en la consulta: " << mysql_error(conn) << endl;
    }

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
    
    // Cerrar la conexión a la base de datos
    mysql_close(conn);
    return 0;

 //-----------------*---------------------*------------------------*--------------------------*-------------------------*---------------------------*	
}