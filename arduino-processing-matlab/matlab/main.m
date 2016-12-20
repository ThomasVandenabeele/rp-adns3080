fclose(instrfind);
close all
s = serial('/dev/tty.usbmodem1421');
s.BaudRate = 115200;

fopen(s);

s.ReadAsyncMode='continuous';

row = 1;
col = 1;
image = zeros(30,30);
NewFrame = imread('foto1.jpg');
figure('Name','ADNS 3080 Video Stream','NumberTitle','off')
%hAx  = axes;
while(s.BytesAvailableFcnCount > 0)
    line = fscanf(s);
    
    if (~strcmp(line, ''))
        if (strcmp(line(1:1), 'S') || strcmp(line(1:1), 'E'))
            
            OldFrame = NewFrame;
            
            row = 1;
            img = mat2gray(image);
            
            NewFrame = imrotate(img,90,'bilinear','crop');
            
            frame = imcrop(OldFrame,[10 10 10 10]);
            c = normxcorr2(frame, NewFrame);
            
            [ypeak, xpeak] = find(c==max(c(:)));
            yoffSet = 10-(ypeak-size(frame,1)+1);
            xoffSet = 10-(xpeak-size(frame,2)+1);
            
            ResizedImage = imresize(NewFrame, 15, 'nearest');
            
            imshow(ResizedImage)
            title(['X_{OFFSET}: ' num2str(xoffSet) '     -     Y_{OFFSET}: ' num2str(yoffSet)]);
            drawnow;
            
        elseif(strcmp(line(1:1), 'I'))
            
        else
            values = strsplit(line, '-');
            if((size(values, 2) > 30) && (row <= 30))
                for i = 1:1:30
                    image(row, i) = str2double(values(1, i));
                end
                row = row + 1;
            end
        end
        
    end
    
end

clear s;
snew = instrfind;
fclose(snew);
