// cheatsheets.h: a title plus up to 7 commands per screen.
// Each command fits in about 40 characters.
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
    "aws s3 cp FILE s3://BUCKET/",
    "aws logs tail GROUP --follow",
  }},
  { "Docker", {
    "docker ps -a",
    "docker start NAME",
    "docker stop NAME",
    "docker logs -f NAME",
    "docker exec -it NAME sh",
    "docker compose up -d",
    "docker compose down",
  }},
};
const int CHEATSHEET_COUNT = sizeof(CHEATSHEETS) / sizeof(CHEATSHEETS[0]);
