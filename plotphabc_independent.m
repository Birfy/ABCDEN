fid=fopen('pha.dat','r');
a=fscanf(fid,'%f %f %f %f',[4,inf]);
a=a';

Nx=64;
Ny=64;
Nz=1;

pha1(1:Nx,1:Ny,1:Nz)=0.0;
phb1(1:Nx,1:Ny,1:Nz)=0.0;

dx=1;
dy=1.732;
dz=1;

n=2;

[X,Y,Z]=meshgrid(dy:dy:Ny*dy*n,dx:dx:Nx*dx*n,dz:dz:Nz*dz*n);

for k=1:Nz
    for i=1:Nx
        for j=1:Ny
            pha1(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,1);
            phb1(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,2);
        end 
    end
end


pha=zeros(Nx*n,Ny*n,Nz*n);
phb=zeros(Nx*n,Ny*n,Nz*n);
for l=1:n
    for m=1:n
        for o=1:n
            for k=1:Nz
                for i=1:Nx
                    for j=1:Ny
                        pha(i+Nx*(l-1),j+Ny*(m-1),k+Nz*(o-1))=pha1(i,j,k);
                        phb(i+Nx*(l-1),j+Ny*(m-1),k+Nz*(o-1))=phb1(i,j,k);
                    end
                end
            end
        end
    end
end

pa = patch(isosurface(X,Y,Z,pha,0.50));
patch(isocaps(X,Y,Z,pha,0.5),'facealpha',0.5,...
    'FaceColor','blue',...
    'EdgeColor','none',...
    'AmbientStrength',.2,...
    'SpecularStrength',.5,...
    'DiffuseStrength',.3);
isonormals(X,Y,Z,pha,pa)

set(pa,'FaceColor','blue','EdgeColor','none')

pb = patch(isosurface(X,Y,Z,phb,0.5));
patch(isocaps(X,Y,Z,phb,0.5),'facealpha',0.5,...
    'FaceColor','green',...
    'EdgeColor','none',...
    'AmbientStrength',.2,...
    'SpecularStrength',.5,...
    'DiffuseStrength',.3);
isonormals(X,Y,Z,phb,pb)
set(pb,'FaceColor','green','EdgeColor','none') %transparency

axis off
daspect([1 1 1]) %set the axial ratio
view([1 1 1]); axis tight
camlight(10,90);lighting phong
light;
light;
% hold on
% subplot(2,3,1)
% contourf(phc,'linestyle','none')
