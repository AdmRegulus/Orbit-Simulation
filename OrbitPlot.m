clc;
clear all;
hold on;

data = csvread('C://Example/File/Path/SimulationResult.csv');

plot(data(:,1), data(:,2), 'yellow');
plot(data(:,4), data(:,5), 'black');
plot(data(:,7), data(:,8), 'magenta');
plot(data(:,10), data(:,11), 'blue');
plot(data(:,13), data(:,14), 'green');
plot(data(:,16), data(:,17), 'cyan');
daspect([1 1 1])
