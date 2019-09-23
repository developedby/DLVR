#include <iostream>
#include <fstream>
#include <wiringPi.h>
#include <raspicam/raspicam.h>

int main ()
{
	wiringPiSetupSys();
	raspicam::RaspiCam cam;
	cam.setRotation(270);
	if (!cam.open())
	{
		
		std::cerr << "Erro ao abrir camera" << std::endl;
		return -1;
	}
	delay(2000);  // Espera um pouco pra camera estabilizar
	cam.grab();  // Manda a camera tirar uma foto
	unsigned char * img = new unsigned char[
		cam.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB)];
	cam.retrieve(img, raspicam::RASPICAM_FORMAT_IGNORE);  // Copia a foto para a variavel
	std::ofstream out("teste.ppm", std::ios::binary);  // Cria o arquivo de saida
	out << "P6\n" << cam.getWidth() << " " << cam.getHeight() << " 255\n";  // Escreve o cabeçalho
	out.write((char*)img, cam.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB));  // Escreve a imagem
	delete img;
	return 0;
}
