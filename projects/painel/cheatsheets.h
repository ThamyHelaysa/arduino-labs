// cheatsheets.h: título + até 7 comandos por tela.
// Cada comando cabe com até ~40 caracteres.
struct CheatSheet {
  const char* title;
  const char* lines[7];
};

const CheatSheet CHEATSHEETS[] = {
  { "AWS", {
    "aws configure",
    "aws configure list",
    "aws sso login",
    "aws s3 ls",
    "aws s3 cp ARQ s3://BUCKET/",
    "aws logs tail GRUPO --follow",
  }},
  { "Docker", {
    "docker ps -a",
    "docker start NOME",
    "docker stop NOME",
    "docker logs -f NOME",
    "docker exec -it NOME sh",
    "docker compose up -d",
    "docker compose down",
  }},
};
const int CHEATSHEET_COUNT = sizeof(CHEATSHEETS) / sizeof(CHEATSHEETS[0]);