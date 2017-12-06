clc;
clear all;

PATH = 'C://Users/Jonah/Pro/Orbits/GitHub/v0.9/SimulationResult.csv';

data = csvread(PATH);

sz = size(data);
objects = sz(2) / 3;

figure
plot3(data(:,1), data(:,2), data(:,3));
hold on;

for x = 1:(objects - 1)
    plot3(data(:, 3 * x + 1), data(:, 3 * x + 2), data(:, 3 * x + 3)); 
end

daspect([1 1 1]);
pbaspect([1 1 1]);