#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <raspicam/raspicam.h>

int main ()
{
	wiringPySysSetup();
	raspicam::RaspiCam cam;
	if (!cam.open())
	{
		
		std::cerr << "Erro ao abrir camera"" << std::endl;
		return -1;
	}
	delay(2000);  // Espera um pouco pra camera estabilizar
	cam.grab();  // Manda a camera tirar uma foto
	unsigned char * img = new unsigned char[
		cam.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];
	cam.retrieve(img, raspicam::RASPICA<+FORMAT_RGB);  // Copia a foto para a variavel
	std::ofstream out("teste.ppm", std::ios::binary);  // Cria o arquivo de saida
	out << "P6\n" << cam.getWidth() << " " << cam.getHeight() << " 255\n";  // Escreve o cabeçalho
	out.write((char*)data, cam.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));  // Escreve a imagem
	delete data;
	return 0;
}
