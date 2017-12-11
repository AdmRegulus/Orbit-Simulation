clc;
clear all;

PATH = 'C://your/path/here/';

Files = dir(strcat(PATH, '*.csv'));
figure
for i = 1:length(Files)
   data = csvread(strcat(PATH, Files(i).name));
   plot3(data(:,1), data(:,2), data(:,3));
   hold on;
end
daspect([1 1 1]);
pbaspect([1 1 1]);