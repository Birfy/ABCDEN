writor = VideoWriter('L2L.mp4', 'MPEG-4');
open(writor);
for i = 0:50:4450
    fid=fopen(strcat('pha',num2str(i),'.dat'),'r');
    a=fscanf(fid,'%f %f %f %f',[4,inf]);
    a=a';
    fclose(fid);

    Nx=16;
    Ny=16;
    Nz=16;

    pha(1:Nx,1:Ny,1:Nz)=0.0;
    phb(1:Nx,1:Ny,1:Nz)=0.0;

    for i=1:Nx
        for j=1:Ny
            for k = 1:Nz
                pha(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,1);
                phb(i,j,k)=a((i-1)*Ny*Nz+(j-1)*Nz+k,2);
            end
        end 
    end
    pa = patch(isosurface(X,Y,Z,pha,0.480));
    patch(isocaps(X,Y,Z,pha,0.50),'facealpha',0,...
        'FaceColor','blue',...
        'EdgeColor','none',...
        'AmbientStrength',.2,...
        'SpecularStrength',.5,...
        'DiffuseStrength',.3);
    isonormals(X,Y,Z,pha,pa)

    set(pa,'FaceColor','blue','EdgeColor','none')

    pb = patch(isosurface(X,Y,Z,phb,0.48));
    patch(isocaps(X,Y,Z,phb,0.50),'facealpha',0,...
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
    frame = getframe;
    close;
    writeVideo(writor, frame);
end
close(writor);