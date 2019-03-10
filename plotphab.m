% Isosurface program for AB diblock copolymer
% Birfy June 2018

% Read file from pha.dat
fid=fopen('pha.dat','r');
a=fscanf(fid,'%f %f %f %f',[4,inf]);
a=a';

% Define the size of the cubic
Nx=90;
Ny=96;
Nz=1;

pha1(1:Nx,1:Ny,1:Nz)=0.0;
phb1(1:Nx,1:Ny,1:Nz)=0.0;
% phc1(1:Nx,1:Ny,1:Nz)=0.0;


dx=0.3;
dy=0.3;
dz=0.3;

% Draw how many times along the axises
n=2;

[X,Y,Z]=meshgrid(dy:dy:Ny*dy*n,dx:dx:Nx*dx*n,dz:dz:Nz*dz*n);

% Reshape the one dimension array to three-dimensions
for k=1:Nz
    for i=1:Nx
        for j=1:Ny
            pha1(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,1);
            phb1(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,2);
            % phc1(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,3);
        end 
    end
end

% Copy the array for n times
pha=zeros(Nx*n,Ny*n,Nz*n);
phb=zeros(Nx*n,Ny*n,Nz*n);
% phc=zeros(Nx*n,Ny*n,Nz*n);
for l=1:n
    for m=1:n
        for o=1:n
            for k=1:Nz
                for i=1:Nx
                    for j=1:Ny
                        pha(i+Nx*(l-1),j+Ny*(m-1),k+Nz*(o-1))=pha1(i,j,k);
                        phb(i+Nx*(l-1),j+Ny*(m-1),k+Nz*(o-1))=phb1(i,j,k);
                        % phc(i+Nx*(l-1),j+Ny*(m-1),k+Nz*(o-1))=phc1(i,j,k);
                    end
                end
            end
        end
    end
end

pa = patch(isosurface(X,Y,Z,pha,0.5));
patch(isocaps(X,Y,Z,pha,0.5),'facealpha',0.5,...
    'FaceColor','blue',...
    'EdgeColor','none',...
    'AmbientStrength',.2,...
    'SpecularStrength',.5,...
    'DiffuseStrength',.3);
isonormals(X,Y,Z,pha,pa)
set(pa,'FaceColor','blue','EdgeColor','none')%transparency

pb = patch(isosurface(X,Y,Z,phb,0.5));
patch(isocaps(X,Y,Z,phb,0.5),'facealpha',0.5,...
    'FaceColor','green',...
    'EdgeColor','none',...
    'AmbientStrength',.2,...
    'SpecularStrength',.5,...
    'DiffuseStrength',.3);
isonormals(X,Y,Z,phb,pb)
set(pb,'FaceColor','green','EdgeColor','none') %transparency

% pc = patch(isosurface(X,Y,Z,phc,0.5));
% patch(isocaps(X,Y,Z,phc,0.5),'facealpha',0.5,...
%     'FaceColor','red',...
%     'EdgeColor','none',...
%     'AmbientStrength',.2,...
%     'SpecularStrength',.5,...
%     'DiffuseStrength',.3);
% isonormals(X,Y,Z,phc,pc)
% set(pc,'FaceColor','red','EdgeColor','none') %transparency

axis off
daspect([1 1 1]) % Set the axial ratio
view([1 1 1]); axis tight % Set the view point
camlight(10,90);lighting gouraud
light;
light;