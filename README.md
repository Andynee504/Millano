# **Projeto Mellano**

## Primeiros passos

Instale o Git **[(Git for Windows)](https://github.com/git-for-windows/git/releases/download/v2.53.0.windows.2/Git-2.53.0.2-64-bit.exe)** marcando "Add Git to PATH". Se preferir a [versão portable](https://github.com/git-for-windows/git/releases/download/v2.51.0.windows.1/PortableGit-2.51.0-64-bit.7z.exe) (para usar também nos computadores sem acesso administrativo).

Abra o prompt de comando (CMD), de preferencia como administrador, e verifique se está devidamente instalado com o comando:

```
git --version
```

Configure sua identidade no primeiro acesso com os comandos:

```
git config --global user.name "Seu Nome"
git config --global user.email "seu@email.com"
```

Se em um computador sem acesso administrativo não tiver Git instalado e não puder usar a versão portable: só resta baixar ZIP pelo navegador e fazer commits/push de outra máquina sendo necessário pen drive/upload manual.

## Configurando Ambiente de Trabalho

### **Importante:**
Evite nomes com acentos, espaços ou caracteres especiais nas pastas e arquivos, pois isso pode causar problemas com o Unity ou scripts.

Escolha o local que quer que o projeto seja salvo, de preferência o mais próximo da raiz possível `(C:)` (assim evitando possiveis erros de falha na localização entre pastas pelos aplicativos a serem usados).

Se solicitado pelo sistema, abra o prompt de comando (CMD) como administrador e, para navegar para a pasta do projeto, basta copiar o endereço da pasta numa janela separada (exemplo: `C:\Users\SeuUsuário\ProjetosUnity\`) e no CMD escrever o comando:

```
cd "endereço copiado"
```

Você pode também, na pasta do projeto e com o clique do botão direito, escolher **"Abrir janela de comando aqui"**.

Agora no CMD basta digitar o comando abaixo:

```bash
git clone https://github.com/Andynee504/Mellano.git
```

Isso criará uma nova pasta chamada `Mellano` com todos os arquivos do projeto.  
Abra essa pasta no Unity (versão recomendada: **6000.3.6f1**) para começar a trabalhar.

## Submetendo alterações

**Execute o comando `git pull` no CMD toda vez que começar a trabalhar no projeto para atualizações não se perderem e para que suas alterações não sejam perdidas.**

Na Unity, depois que terminar uma atualização, vá no menu `Tools > Version > Bump Minor` para aplicar a versão nova. (Bump PATCH é **APENAS** para criação/implementação de novas funcionalidades)

Para salvar e enviar as alterações para *Branch* (github main) execute os comandos:

1. `git status`
  - Para conferir arquivos alterados e se está no projeto certo. Se não aparecer `Your branch is up to date with 'origin/main'.` consulte os colegas em como prosseguir.
2. `git add .`
  - Para adicionar as alterações.
3. `git commit -m "detalhacao das alteracoes"`
  - Para criar o log de atividade (*changelog*).
4. `git push`
  - Para enviar as alterações para *main*.

### **IMPORTANTE:**
**Lembre-se de abrir o prompt de comandos (CMD) na pasta do projeto ou abrir como administrador e navegar até a pasta seguindo os passos anteriores.**