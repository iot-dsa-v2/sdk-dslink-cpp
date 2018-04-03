FROM base/archlinux

ARG user_id

RUN cp /etc/pacman.d/mirrorlist /etc/pacman.d/mirrorlist.backup
RUN sed -i 's/^#Server/Server/' /etc/pacman.d/mirrorlist.backup
RUN rankmirrors -n 6 /etc/pacman.d/mirrorlist.backup > /etc/pacman.d/mirrorlist

RUN yes | pacman -Syu
RUN yes | pacman --needed -S boost boost-libs gcc ninja openssl cmake make git

RUN mkdir -p /app
WORKDIR /app

RUN useradd -r -u $user_id broker

USER broker

EXPOSE 8080
