close all
picture   = imread('foto2.jpg'); %Second frame
I = imread('foto1.jpg');

hFig1 = figure;
hAx1  = axes;
imshow(I,'Parent', hAx1);
imrect(hAx1, [150 150 150 150]);

frame = imcrop(I,[150 150 150 150]);
%imshow(frame)
figure, imshowpair(picture,frame,'montage')

c = normxcorr2(frame, picture);
figure, surf(c), shading flat

[ypeak, xpeak] = find(c==max(c(:)));
yoffSet = ypeak-size(frame,1);
xoffSet = xpeak-size(frame,2);

hFig = figure;
hAx  = axes;
imshow(picture,'Parent', hAx);
hold on;
rectangle('Position', [10*15, 10*15, 10*15, 10*15], 'EdgeColor', 'b');
hold on;
rectangle('Position', [xoffSet+1, yoffSet+1, size(frame,2), size(frame,1)], 'EdgeColor', 'r');
%hAx.Color = 'blue';
%imrect(hAx, [10*15, 10*15, 10*15, 10*15]);
%hAx.Color = 'red';
%imrect(hAx, [xoffSet+1, yoffSet+1, size(frame,2), size(frame,1)]);