# Compactador_Imagem
Desenvolvemos um sistema de compactação e descompactação de imagens BMP 24 bits. O processo começou com a leitura do arquivo Imagem, onde extraí o cabeçalho e armazenei os dados da imagem em uma matriz. Por limitação de memória, utilizei arquivos com no máximo 11KB.

A compactação foi feita de forma recursiva, dividindo a imagem em quadrantes. Quando cada quadrante alcançava 3x3 pixels ou menos, escolhia-se o pixel central para representá-lo. Os valores RGB desses pixels foram armazenados em três vetores (R, G e B). Esses dados, junto com o cabeçalho original, foram gravados no arquivo imagemCompactada.zmp.

Na etapa de reconstrução, o arquivo compactado foi lido. Usando a mesma lógica recursiva de divisão, a imagem foi reconstruída preenchendo as regiões da matriz com os valores RGB armazenados, até gerar o novo arquivo imagemDescompactada.bmp.
