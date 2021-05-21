FROM esphome/esphome-base-amd64:3.0.0

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends sudo bash-completion python3-wheel net-tools \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

RUN pip3 --disable-pip-version-check --no-cache-dir install esphome==1.18.0 pylint black \
 && rm -rf /tmp/pip-tmp

RUN echo "alias ll='ls -l'" >> /etc/bash.bashrc \
 && echo "alias la='ls -A'" >> /etc/bash.bashrc \
 && echo "alias l='ls -CF'" >> /etc/bash.bashrc

RUN useradd -m -s /bin/bash -G sudo vscode
USER vscode

RUN curl https://raw.githubusercontent.com/esphome/esphome/dev/platformio.ini -s -o /home/vscode/platformio.ini
RUN cd /home/vscode && pio run -t compiledb -e livingroom32 -e livingroom8266 && pio system prune -f
