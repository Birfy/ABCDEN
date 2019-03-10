writor = VideoWriter('R2LL18.mp4', 'MPEG-4');
open(writor);
for t = 0:1000:96000
    fid=fopen(strcat('pha',num2str(t),'.dat'),'r');
    a=fscanf(fid,'%f %f %f %f',[4,inf]);
    a=a';
    fclose(fid);

    Nx=64;
    Ny=64;

    pha(1:Nx,1:Ny)=0.0;
    phb(1:Nx,1:Ny)=0.0;

    for i=1:Nx
        for j=1:Ny
            pha(i,j)=a((i-1)*Ny+j,1);
            phb(i,j)=a((i-1)*Ny+j,2);
        end 
    end
    lim=10;
    imagesc(pha);
    frame = getframe;
    writeVideo(writor, frame);
end
close(writor);