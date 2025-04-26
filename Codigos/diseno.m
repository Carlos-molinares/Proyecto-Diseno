% Leer el archivo de texto
filename = 'Final_barrero_real.txt';
fid = fopen(filename, 'rt');

valores = [];

% Leer línea por línea
while ~feof(fid)
    linea = fgetl(fid);

    % Saltar líneas sin datos válidos
    if isempty(linea) || contains(linea, 'Step Information') || contains(linea, 'time')
        continue;
    end

    % Extraer los dos valores si existen
    datos = sscanf(linea, '%f\t%f');

    % Si hay dos datos, añadir solo el segundo (una vez por grupo de 4)
    if numel(datos) == 2
        valores(end+1) = datos(2); %#ok<SAGROW>
        
        % Saltar las siguientes 3 líneas que son repeticiones
        for i = 1:3
            if ~feof(fid)
                fgetl(fid);
            end
        end
    end
end

fclose(fid);

% Calcular estadísticas
media = mean(valores);
desviacion = std(valores);
mediana = median(valores);
minimo = min(valores);
maximo = max(valores);

% Mostrar estadísticas en consola
fprintf('Media: %.6f\n', media);
fprintf('Desviación estándar: %.6f\n', desviacion);
fprintf('Mediana: %.6f\n', mediana);
fprintf('Mínimo: %.6f\n', minimo);
fprintf('Máximo: %.6f\n', maximo);

% Mostrar histograma
figure;
h = histogram(valores, 'BinWidth', 0.001);
xlabel('Valor de Vout para uA');
ylabel('Frecuencia');
title(sprintf('Histograma\nMedia=%.4f, STD=%.4f', media, desviacion));
grid on;

% Líneas para estadísticas con etiquetas visibles
hold on;

ymax = max(h.Values) * 1.05; % Altura para colocar etiquetas


% Mínimo
xline(minimo, 'g-', 'LineWidth', 1.2);
text(minimo, ymax, sprintf('Mínimo = %.4f', minimo), 'Color', 'g', 'HorizontalAlignment','center', 'VerticalAlignment','bottom');

% Máximo
xline(maximo, 'r-', 'LineWidth', 1.2);
text(maximo, ymax, sprintf('Máximo = %.4f', maximo), 'Color', 'r', 'HorizontalAlignment','center', 'VerticalAlignment','bottom');

hold off;
