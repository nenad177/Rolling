#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "SOIL.h"

#define TIMER_ID_BALL 0
#define TIMER_INTERVAL_BALL 20
#define TIMER_ID 0
#define TIMER_INTERVAL 17

static int faster = 0; // Svakoj prepreci dodeljuje nasumicnu brzinu.
static int start = 1; // Omogucava da se ne ispisuje "PAUSED" pre pokretanja igre.
static int alive = 1; // Onemogucava nastavak igre pritiskom na taster 'p' nakon
						// sudara sa preprekom.
static int increase = 1; // Omogucava da se broj poena poveca samo
							// jednom nakon preskocene prepreke.
static int score = 0; // Broj poena.
static int highscore = 0; // Najbolji rezultat.
static int new_high_score = 0; // Oznacava da li je popravljen najbolji rezultat.
static int higher = 0; // Da li lopta skace vise (dupli pritisak space tastera).
static float speed = 0; // Brzina igre.
static int current_obstacle; // Tip prepreke koja se sledeca pojavljuje.
static int fullscreen = 0;
static int play = 0; // Pokretanje i pauziranje igre.
static int w_width, w_height;
static float cam_x = -3, cam_y = 2, cam_z = 10;
static float jump = 0, height = 2; // Promenljive za animaciju skoka lopte.
static float obstacle_x = 0; // x koordinata prepreke.
static float ball_x = -5; // x koordinata centra lopte.
static float ball_y = 0.75; // y koordinata centra lopte.
float x_1, y_1, x_2, y_2; // Koordinate centara lopte i prepreke.
static int animation_ongoing_ball = 0;
float ground = 0; // Promenljiva za kretanje podloge po x osi.

/* Promenljiva back se koristi za iscrtavanje lopte na odredjenoj poziciji
    kada dodje do sudara, jer inace se lopta i prepreka preklapaju u
    trenutku kada se animacija zaustavi. */
float back = 0;
GLuint texture[4]; // Niz sa teksturama.



/* Deklarisanje callback funkcija. */
static void on_keyboard(unsigned char key, int x, int y);
static void on_display(void);
static void on_timer_ball(int value);
static void on_timer_obstacles(int value);
static void on_reshape(int width, int height);
// Funkcija za crtanje razlicitih tipova prepreka.
void draw_obstacle(float obstacle_x, int type); 
// Funkcije za crtanje lopte, podloge i pozadine.
void draw_background();
void draw_ground();
void draw_ball();
/* Funkcija koja detektuje sudar izmedu lopte i prepreke. */
int collision(float x1, float y1, float x2, float y2, int type);
 // F-ja vraca apsolutnu razliku 2 broja.
float abs_diff(float a, float b);
// Funkcija koja ucitava teksture u niz texture.
void loadTextures();
/* Izmenjene funkcije glutSolidCube i drawBox tako da se moze postaviti tekstura
    na objekat nacrtan ovom funkcijom. 
    Parametri x i y - za ponavljanje teksture po x i y osi.
    Patametar t - indeks niza texture.
    Ideja za ovo resenje: https://stackoverflow.com/questions/327043/how-to-apply-texture-to-glutsolidcube */
void APIENTRY glutSolidCube_t(GLdouble size, float x, float y, int t);
// Funkcija koju poziva glutSolidCube_t i prosleduje joj odgovarajuce parametre.
static void drawBox(GLfloat size, GLenum type, float x, float y, int t);
/* Funkcija ispisuje treutni broj poena. */
void displayScore(float x, float y, float z, int type);
/* Funkcija ispisuje poruku u slucaju kraja igre. */
void displayGameOver();
/* Funkcija ispisuje najbolji rezultat ili poruku da je on ostvaren */
void displayHighScore();
// Ispisuje tekst kada je igra pauzirana.
void displayPause();
// Ispisuje tekst pre pocetka igre.
void displayStart();
// Resetovanje parametara.
void reset();

int main(int argc, char **argv)
{
    /* Inicijalizuje se glut. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    /* Tip prepreke moze biti 1, 2, 3 ili 4 */
    srand(time(NULL));
    current_obstacle = (rand() % 4) + 1;

    /* Kreiranje prozora. */
    glutInitWindowSize(950, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);

    loadTextures();
    
    /* Registruju se callback funkcije. */
    glutKeyboardFunc(on_keyboard);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);

    /* OpenGL inicijalizacija. */
    glClearColor(0.031, 0.031, 0.031, 0);
    glEnable(GL_DEPTH_TEST);

    /* Glavna petlja. */
    glutMainLoop();

    return 0;   
}

static void on_keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        // Zavrsava se program.
        exit(0);
        break;

    case 'r':
    case 'R':
    	// Restartuje se igra.
    	if(!alive) {
	    	reset();
	    	glutTimerFunc(TIMER_INTERVAL - speed, on_timer_obstacles, TIMER_ID);
	    }
	    break;

	case 'f':
	case'F':
		// Ulaz i izlaz iz fullscreen-a.
	    if(!fullscreen){
	        fullscreen = 1;
	        glutFullScreen();
	    } 
	    else {
	        fullscreen = 0;
	        // Ideja za sledece dve funkcije:
	        // https://stackoverflow.com/questions/13083967/exiting-glutfullscreen
	        glutReshapeWindow(950, 600);
	        glutPositionWindow(100, 100);
	    }
	    break;

    case 'p':
    case 'P':
    	if(alive) {
    		start = 0;
	        // Pokrece se animacija.
	        if(!play) {
	            glutTimerFunc(TIMER_INTERVAL - speed - faster, on_timer_obstacles, TIMER_ID);
	            play = 1;
	        }
	        else
	            play = 0; // Zaustavlja se animacija.
	    }
        break;

    case 32:
        if(animation_ongoing_ball && higher == 0) {
            higher = 1;
        }
        // Pokrece se animacija skoka lopte.
        if(!animation_ongoing_ball && play == 1) {
            glutTimerFunc(TIMER_INTERVAL_BALL - speed/2, on_timer_ball, TIMER_ID_BALL);
            animation_ongoing_ball = 1;
        }
        break;
    }
}

static void on_timer_ball(int value)
{

    /* Provera da li je u pitanju odgovarajuci tajmer. */
    if(value != TIMER_ID_BALL)
        return;
    
    /* Koeficijent height se povecava ako je registrovan dvostruki pritisak
        space tastera. */
    if(higher && height <= 3.2)
        height += 0.1;

    // Lopta se malo zadrzava na vrhu skoka zbog lakseg preskakanja prepreke.
    if(ball_y + height*sin(jump) < 2.65)
        jump += 0.1;
    else
        jump += 0.05;

    /* Ako se lopta vratila na podlogu prekida se animacija. */
    if(sin(jump) <= 0) {
        jump = 0;
        animation_ongoing_ball = 0;
        higher = 0;
        height = 2;
    }
    /* Ponovno iscrtavanje */
    glutPostRedisplay();

    if (animation_ongoing_ball) {
        glutTimerFunc(TIMER_INTERVAL_BALL - speed, on_timer_ball, TIMER_ID_BALL);
    }
}

static void on_timer_obstacles(int value)
{
    // Provera da li je u pitanju odgovarajuci tajmer.
    if(value != TIMER_ID)
        return;
    // Ogranicava se kretanje podloge.
    if(ground >= 1)
      ground = 0;
      
    // Pomeranje prepreke i podloge.
    if(play) {
        obstacle_x += 0.1;
        ground = ground + 0.1;
      }

    /* Kada prepreka "izadje" iz prozora na levoj strani, vraca se
        na pocetnu poziciju kako bi se ponovo pojavila. */
    if(obstacle_x >= 24) {
        obstacle_x = 0;
        increase = 1;
        faster = rand() % 5; // Racuna se random brzina za svaku prepreku.
        /* Odreduje se tip sledece prepreke. */
        current_obstacle = (rand() % 4) + 1;
        if(speed <= 9)
            speed += 0.2; // Ubrzava se igra do odredjene brzine.
    }
    // Uvecava se broj poena kada je preskocena prepreka.
    if(obstacle_x > 18 && increase) {
    	if(current_obstacle == 4)
    		score += 3;
    	else if(current_obstacle == 2)
    		score += 2;
    	else
    		score += 1;
    	increase = 0;
    }

    /* Racunaju se koordinace centara lopte i prepreke. */
    x_1 = ball_x;
    y_1 = ball_y + height*sin(jump);
    x_2 = 10.5 - obstacle_x;
    if(current_obstacle == 1)
        y_2 = 0.75;
    else if(current_obstacle == 3)
        y_2 = 0.85;
    else if(current_obstacle == 2)
        y_2 = 1.4;
    else 
        y_2 = 1.45;

    /* Ukoliko je doslo do sudara, igra se zaustavlja i azurira se
    		najbolji rezultat ako je potrebno. */
    if(collision(x_1, y_1, x_2, y_2, current_obstacle)) {
        play = 0;
        animation_ongoing_ball = 0;
        alive = 0;
        if(score > highscore) {
        	new_high_score = 1;
        	highscore = score;
        }
    }

    /* Ponovno iscrtavanje */
    glutPostRedisplay();

    if(play) {
        glutTimerFunc(TIMER_INTERVAL - speed - faster, on_timer_obstacles, TIMER_ID);
    }
}

static void on_reshape(int width, int height)
{
    w_width = width;
    w_height = height;

    glViewport(0, 0, w_width, w_height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float) w_width/w_height, 1, 50);
}

static void on_display(void)
{
    // Brise se prethodni sadrzaj prozora.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Podesava se projekcija.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
            60,
            w_width/(float)w_height,
            1, 50);

    // Podesava se tacka pogleda.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
            cam_x, cam_y, cam_z,
            -2, 0, 0,
            0, 1, 0
        );
    

	draw_ground();
	draw_ball();
   
    // Crta se prepreka, f-ji se prosledjuje x koordinata i tip prepreke.
    draw_obstacle(obstacle_x, current_obstacle);

    draw_background();

    /* Broj poena se pikazuje u gornjem levom uglu za vreme igre, a kada
    		je igra zavrsena onda na sredini. */
    if(start)
    	displayStart();
    if(alive && !play && !start)
    	displayPause();
    if(alive && !start)
    	displayScore(-0.9 + cam_x, 0.9 + cam_y, 0 + cam_z, 1);
    else if(!alive)	{
    	displayGameOver();
    	displayHighScore();
    	displayScore(-0.19 + cam_x, 0.29 + cam_y, 0.5 + cam_z, 2);
    }
    // Nova slika se salje na ekran.
    glutSwapBuffers();
}

void draw_background()
{
	// Crtanje cetvorougla i postavljanje teksture na njega (ovo je pozadina).
    glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
		glScalef(2.4, 2.4, 1);
		glTranslatef(0.09, -0.17, 0);
        glBegin(GL_POLYGON);
            glTexCoord2f(0, 0);
            glVertex3f(-0.5 + cam_x, -0.5 + cam_y, -0.95 + cam_z);
            glTexCoord2f(0, 1);
            glVertex3f(-0.5 + cam_x, 0.5 + cam_y, -0.99 + cam_z);
            glTexCoord2f(1, 1);
            glVertex3f(0.5 + cam_x, 0.5 + cam_y, -0.9 + cam_z);
            glTexCoord2f(1, 0);
            glVertex3f(0.5 + cam_x, -0.5 + cam_y, -0.85 + cam_z);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void draw_ground()
{
	// Crtanje podloge na kojoj se lopta nalazi.
    glPushMatrix();
    	glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glTranslatef(-ground, 0, -0.2);
        glScalef(50, 0.5, 4);
        glutSolidCube_t(1, 50, 0.5, 0);
    glPopMatrix();
}

void draw_ball()
{
	//Crtanje lopte.
	glPushMatrix();
        // Podesavanje osvetljenja i materijala za loptu.
        GLfloat light_position_ball[] = { 1, 1, 1, 0 };
        GLfloat light_ambient_ball[] = { 0.7, 0.7, 0, 1 };
        GLfloat light_diffuse_ball[] = { 0.7, 0.7, 0, 1 };
        GLfloat light_specular_ball[] = { 0.5, 0.5, 0.5, 1 };

        GLfloat ambient_coeffs_ball[] = { 0.4, 0.4, 0.4, 1 };
        GLfloat diffuse_coeffs_ball[] = { 0.8, 0.8, 0.0, 1 };
        GLfloat specular_coeffs_ball[] = { 1, 1, 1, 1 };
        GLfloat shininess_ball = 20;

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position_ball);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_ball);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_ball);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular_ball);

        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_coeffs_ball);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_coeffs_ball);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular_coeffs_ball);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess_ball);
        
        /* Ako je back > 0, lopta se iscrtava na odredjenoj "visini", da se
            ne bi preklapala sa preprekom kada dodje do sudara. */
        if(back > 0)
            glTranslatef(ball_x, back, 0);
        else
            glTranslatef(ball_x, ball_y + height*sin(jump), 0);
        glRotatef(obstacle_x*40, 0, 0, -1);
        glutSolidSphere(0.5, 15, 15);
    glPopMatrix();
}

void draw_obstacle(float obstacle_x, int type)
{
    glPushMatrix();

        // Podesavanje osvetljenja i materijala za prepreke.
        if(type == 1 || type == 2) {
          GLfloat light_position_obs[] = { -3, 2, 0.5, 0 };
          GLfloat light_ambient_obs[] = { 0.38, 0.27, 0.23, 1 };
          GLfloat light_diffuse_obs[] = { 0.38, 0.27, 0.23, 1 };
          GLfloat light_specular_obs[] = { 0.38, 0.27, 0.23, 1 };

          GLfloat ambient_coeffs_obs[] = { 0.38, 0.27, 0.23, 1 };
          GLfloat diffuse_coeffs_obs[] = { 0.5, 0.5, 0.4, 1 };
          GLfloat specular_coeffs_obs[] = { 1, 1, 1, 1 };
          GLfloat shininess_obs = 10;

          glEnable(GL_LIGHTING);
          glEnable(GL_LIGHT0);
          glLightfv(GL_LIGHT0, GL_POSITION, light_position_obs);
          glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_obs);
          glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_obs);
          glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular_obs);

          glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_coeffs_obs);
          glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_coeffs_obs);
          glMaterialfv(GL_FRONT, GL_SPECULAR, specular_coeffs_obs);
          glMaterialf(GL_FRONT, GL_SHININESS, shininess_obs);
        }
        else {
          GLfloat light_position_obs[] = { -3, 2, 0.5, 0 };
          GLfloat light_ambient_obs[] = { 0.050, 0.325, 0.015, 1 };
          GLfloat light_diffuse_obs[] = { 0.050, 0.325, 0.015, 1 };
          GLfloat light_specular_obs[] = { 0.050, 0.325, 0.015, 1 };

          GLfloat ambient_coeffs_obs[] = { 0.050, 0.325, 0.015, 1 };
          GLfloat diffuse_coeffs_obs[] = { 0.5, 0.5, 0.4, 1 };
          GLfloat specular_coeffs_obs[] = { 1, 1, 1, 1 };
          GLfloat shininess_obs = 10;

          glEnable(GL_LIGHTING);
          glEnable(GL_LIGHT0);
          glLightfv(GL_LIGHT0, GL_POSITION, light_position_obs);
          glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient_obs);
          glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_obs);
          glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular_obs);

          glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_coeffs_obs);
          glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_coeffs_obs);
          glMaterialfv(GL_FRONT, GL_SPECULAR, specular_coeffs_obs);
          glMaterialf(GL_FRONT, GL_SHININESS, shininess_obs);
        }
        // Cetiri tipa prepreka.
        if(type == 1) {
            glTranslatef(10.5 - obstacle_x, 0.75, 0);
            glutSolidCube_t(1, 1, 1, 1);
        }
        else if(type == 2) {
            glTranslatef(10.5 - obstacle_x, 1.40, 0);
            glScalef(1, 2.3, 1);
            glutSolidCube_t(1, 1, 2, 1);
        }
        else if(type == 3) {
            glTranslatef(10.5 - obstacle_x, 0.85, 0);
            glutSolidSphere(0.6, 30, 30);
        }
        else {
            glTranslatef(10.5 - obstacle_x, 1.45, 0);
            glutSolidSphere(1.2, 30, 30);
        }
    glPopMatrix();
}

int collision(float x1, float y1, float x2, float y2, int type)
{
    /* Provera sudara lopte i prepreke tipa 1. */
    if(type == 1) {
        if(y1 <= 1.25 && abs_diff(x1, x2) <= 1)
            return 1;
        else if(x1 >= (x2 - 0.6) && x1 <= (x2 + 0.6) && abs_diff(y1, y2) <= 1) {
            back = 1.75;
            return 1;
        }
        else if(abs_diff(y1, 1.25) <= 0.2 && abs_diff(x1, x2) <= 1)
            return 1;
    }

    /* Provera sudara lopte i prepreke tipa 2. */
    else if(type == 2) {
        if(y1 <= 2.55 && abs_diff(x1, x2) <= 1)
            return 1;
        else if(x1 >= (x2 - 0.6) && x1 <= (x2 + 0.6) && abs_diff(y1, y2) <= 1.65) {
            back = 3.05;
            return 1;
        }
        else if(abs_diff(y1, 2.55) <= 0.2 && abs_diff(x1, x2) <= 1)
            return 1;
    }

    /* Provera sudara lopte i prepreke tipa 3. */
    else if(type == 3) {
        if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) <= 1.1) {
            if(jump > 0)
                jump -= 0.1;
            return 1;
        }
    }

    /* Provera sudara lopte i prepreke tipa 4. */
    else if(type == 4) {
        if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) <= 1.7) {
            return 1;
        }
    }

    return 0;
}

float abs_diff(float a, float b)
{
    float c = a - b;
    
    if(c < 0)
        c *= -1;

    return c;
}

void loadTextures()
{
    texture[0] = SOIL_load_OGL_texture
    (
        "./Textures/stone.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);

    texture[1] = SOIL_load_OGL_texture
    (
        "./Textures/box.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);

    texture[2] = SOIL_load_OGL_texture
    (
        "./Textures/sky.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);

    texture[3] = SOIL_load_OGL_texture
    (
        "./Textures/gameover.png",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);
}

static void
drawBox(GLfloat size, GLenum type, float x, float y, int t)
{
  static GLfloat n[6][3] =
  {
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0}
  };
  static GLint faces[6][4] =
  {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };
  GLfloat v[8][3];
  GLint i;

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    
   for (i = 5; i >= 0; i--) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[t]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2f(0, 0);
        glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2f(0, y);
        glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2f(x, y);
        glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2f(x, 0);
        glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
    glDisable(GL_TEXTURE_2D);
  }
}

void APIENTRY
glutSolidCube_t(GLdouble size, float x, float y, int t)
{
  drawBox(size, GL_QUADS, x, y, t);
}

void displayScore(float x, float y, float z, int type){
    glPushMatrix();
    	glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
        glDisable(GL_LIGHTING);
        glColor3f(0.7, 0.7, 0);
        glRasterPos3f(x, y, z);
        char dscore[20];
        if(type == 1)
        	sprintf(dscore, "Score: %d", score);
        else
        	sprintf(dscore, "S C O R E:  %d", score);
        int len = strlen(dscore);
        for(int i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, dscore[i]);
        glEnable(GL_LIGHTING);
        glLoadIdentity();
    glPopMatrix();
    glColor3f(1, 1, 1);
}

void displayHighScore()
{
	glPushMatrix();
    	glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
        glDisable(GL_LIGHTING);
        char hscore[35];
        if(!new_high_score) {
	        glColor3f(0.2, 0, 0.4);
	        glRasterPos3f(-0.19 + cam_x, 0.16 + cam_y, 0.5 + cam_z);
	        sprintf(hscore, "High score: %d", highscore);
    	}	
        else {
        	glColor3f(0.9, 0.9, 0.9);
	        glRasterPos3f(-0.35 + cam_x, 0.16 + cam_y, 0.5 + cam_z);
        	sprintf(hscore, "N E W  H I G H  S C O R E !");
        }
        int len = strlen(hscore);
        for(int i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, hscore[i]);
        glEnable(GL_LIGHTING);
        glLoadIdentity();
    glPopMatrix();
    glColor3f(1, 1, 1);
}

void displayGameOver()
{
	 glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glBindTexture(GL_TEXTURE_2D, texture[3]);
        glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
		glColor3f(0.7, 0.0, 0);
        glBegin(GL_POLYGON);
            glTexCoord2f(0, 0);
            glVertex3f(-0.5 + cam_x, -0.5 + cam_y, 0 + cam_z);
            glTexCoord2f(0, 1);
            glVertex3f(-0.5 + cam_x, 0.5 + cam_y, 0 + cam_z);
            glTexCoord2f(1, 1);
            glVertex3f(0.5 + cam_x, 0.5 + cam_y, 0.1 + cam_z);
            glTexCoord2f(1, 0);
            glVertex3f(0.5 + cam_x, -0.5 + cam_y, 0.1 + cam_z);
        glEnd();
        glLoadIdentity();
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

}

void displayPause()
{
	glPushMatrix();
    	glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 1);
        glRasterPos3f(-0.16 + cam_x, 0.7 + cam_y, 0 + cam_z);
        char *paused = "P  A  U  S  E  D";
        int len = strlen(paused);
        for(int i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, paused[i]);
        glEnable(GL_LIGHTING);
        glLoadIdentity();
    glPopMatrix();
    glColor3f(1, 1, 1);
}

void displayStart()
{
	glPushMatrix();
    	glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
        glDisable(GL_LIGHTING);
        glColor3f(0.7, 0.7, 0.7);
        glRasterPos3f(-0.23 + cam_x, 0.7 + cam_y, 0 + cam_z);
        char *start = "P r e s s  P  t o  s t a r t";
        int len = strlen(start);
        for(int i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, start[i]);
        glEnable(GL_LIGHTING);
        glLoadIdentity();
    glPopMatrix();
    glColor3f(1, 1, 1);
}

void reset()
{
	alive = 1;
	play = 1;
	increase = 1;
	score = 0;
	new_high_score = 0;
	higher = 0;
	speed = 0;
	current_obstacle = (rand() % 4) + 1;
	jump = 0;
	height = 2;
	obstacle_x = 0;
	ground = 0;
	back = 0;
}
