class Jogo : public olcConsoleGameEngine
{
public:
	Jogo()
	{
		m_sAppName = L"Asteróides";
	}

private:
	struct sSpaceObject
	{
		int nSize;
		float x;
		float y;
		float dx;
		float dy;
		float angle;
	};

	vector<sSpaceObject> vecAsteroids;
	vector<sSpaceObject> vecBullets;
	sSpaceObject player;
	bool bDead = false;
	int nScore = 0;

	vector<pair<float, float>> vecModelShip;
	vector<pair<float, float>> vecModelAsteroid;

protected:
	// Chamado pelo olcConsoleGameEngine
	virtual bool OnUserCreate()
	{
		vecModelShip =
		{
			{ 0.0f, -5.0f},
			{-2.5f, +2.5f},
			{+2.5f, +2.5f}
		}; // Triângulo isósceles

		// Criar círculos sempre a mexer
		int verts = 20;
		for (int i = 0; i < verts; i++)
		{   //Tirámos o ruído(noise) para transformar em círculos (baseado no jogo do balls)
			//float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
			vecModelAsteroid.push_back(make_pair(sinf(((float)i / (float)verts) * 6.0f),
				cosf(((float)i / (float)verts) * 6.0f)));
		}

		ResetGame();
		return true;
	}

	void ResetGame()
	{
		// Posição inicial
		player.x = ScreenWidth() / 2.0f;
		player.y = ScreenHeight() / 2.0f;
		player.dx = 0.0f;
		player.dy = 0.0f;
		player.angle = 0.0f;

		vecBullets.clear();
		vecAsteroids.clear();

		// Pôr dois asteroides
		vecAsteroids.push_back({ (int)16, 20.0f, 20.0f, 8.0f, -6.0f, 0.0f });
		vecAsteroids.push_back({ (int)16, 100.0f, 20.0f, -5.0f, 3.0f, 0.0f });

		// Sair do jogo
		bDead = false;
		nScore = false;
	}

	// Mudar de direção
	void WrapCoordinates(float ix, float iy, float& ox, float& oy)
	{
		ox = ix;
		oy = iy;
		if (ix < 0.0f)	ox = ix + (float)ScreenWidth();
		if (ix >= (float)ScreenWidth())	ox = ix - (float)ScreenWidth();
		if (iy < 0.0f)	oy = iy + (float)ScreenHeight();
		if (iy >= (float)ScreenHeight()) oy = iy - (float)ScreenHeight();
	}

	// Overriden desenhos rotinas
	virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
	{
		float fx, fy;
		WrapCoordinates(x, y, fx, fy);
		olcConsoleGameEngine::Draw(fx, fy, c, col);
	}

	bool IsPointInsideCircle(float cx, float cy, float radius, float x, float y)
	{
		return sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) < radius;
	}

	// Chamado pelo olcConsoleGameEngine
	virtual bool OnUserUpdate(float fElapsedTime)
	{
		if (bDead)
			ResetGame();

		// Apagar
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

		// Direções
		if (m_keys[VK_LEFT].bHeld)
			player.angle -= 5.0f * fElapsedTime;
		if (m_keys[VK_RIGHT].bHeld)
			player.angle += 5.0f * fElapsedTime;

		// Aplicar aceleração
		if (m_keys[VK_UP].bHeld)
		{
			// Acceleração muda velocidade
			player.dx += sin(player.angle) * 20.0f * fElapsedTime;
			player.dy += -cos(player.angle) * 20.0f * fElapsedTime;
		}

		// Velocidade muda a posição
		player.x += player.dx * fElapsedTime;
		player.y += player.dy * fElapsedTime;

		// Mantendo no espaço do jogo
		WrapCoordinates(player.x, player.y, player.x, player.y);

		// Confirmar colisões
		for (auto& a : vecAsteroids)
			if (IsPointInsideCircle(a.x, a.y, a.nSize, player.x, player.y))
				bDead = true; // Uh oh...

		// Disparos na direção da nave
		if (m_keys[VK_SPACE].bReleased)
			vecBullets.push_back({ 0, player.x, player.y, 50.0f * sinf(player.angle), -50.0f * cosf(player.angle), 100.0f });

		// Desenha astros
		for (auto& a : vecAsteroids)
		{
			// Posição é mudada de acordo com a velocidade
			a.x += a.dx * fElapsedTime;
			a.y += a.dy * fElapsedTime;
			a.angle += 0.5f * fElapsedTime; // rotação diferente

			// localização do asteroide
			WrapCoordinates(a.x, a.y, a.x, a.y);

			// Desenha asteroides
			DrawWireFrameModel(vecModelAsteroid, a.x, a.y, a.angle, (float)a.nSize, FG_YELLOW);

		}

		// Asteroides criados depois de colisão
		vector<sSpaceObject> newAsteroids;

		//Balas
		for (auto& b : vecBullets)
		{
			b.x += b.dx * fElapsedTime;
			b.y += b.dy * fElapsedTime;
			WrapCoordinates(b.x, b.y, b.x, b.y);
			b.angle -= 1.0f * fElapsedTime;

			// Colisão com asteroides
			for (auto& a : vecAsteroids)
			{
				
				if (IsPointInsideCircle(a.x, a.y, a.nSize, b.x, b.y))
				{
					// Se um astro for atingido, desaparece
					// Fazer desaparecer os disparos. Pôr som
					//sndPlaySound(TEXT("sounds\\Thunder.Wav"), SND_ASYNC);
					sndPlaySound(TEXT("sounds\\explosion-02.Wav"), SND_ASYNC);
					b.x = -100;

					// Cria pequenos astros quando são atingidos, ficam mais pequenos
					if (a.nSize > 4)
					{
						float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
						newAsteroids.push_back({ (int)a.nSize >> 1 ,a.x, a.y, 10.0f * sinf(angle1), 10.0f * cosf(angle1), 0.0f });
						newAsteroids.push_back({ (int)a.nSize >> 1 ,a.x, a.y, 10.0f * sinf(angle2), 10.0f * cosf(angle2), 0.0f });
					}

					// Apaga-os da cena
					a.x = -100;
					nScore += 100; 
				}
			}
		}

		// AAdicionar ao vetor
		for (auto a : newAsteroids)
			vecAsteroids.push_back(a);

		// Apaga os asteroides dependendo do número total deles em cena
		if (vecAsteroids.size() > 0)
		{
			auto i = remove_if(vecAsteroids.begin(), vecAsteroids.end(), [&](sSpaceObject o) { return (o.x < 0); });
			if (i != vecAsteroids.end())
				vecAsteroids.erase(i);
		}

		if (vecAsteroids.empty()) // Se desaparecerem todos os asteroides, nível completo e sobe
		{
			// Nível
			nScore += 1000; // Progressão de nível
			vecAsteroids.clear();
			vecBullets.clear();

			// Pôr mais asteroides em sítios onde não colida com a nave.
			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle - 3.14159f / 2.0f) + player.x,
											  30.0f * cosf(player.angle - 3.14159f / 2.0f) + player.y,
											  10.0f * sinf(player.angle), 10.0f * cosf(player.angle), 0.0f });

			vecAsteroids.push_back({ (int)16, 30.0f * sinf(player.angle + 3.14159f / 2.0f) + player.x,
											  30.0f * cosf(player.angle + 3.14159f / 2.0f) + player.y,
											  10.0f * sinf(-player.angle), 10.0f * cosf(-player.angle), 0.0f });
		}

		// Remover disparos que saiem para lá dos limites do ecrã
		if (vecBullets.size() > 0)
		{
			auto i = remove_if(vecBullets.begin(), vecBullets.end(), [&](sSpaceObject o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth() - 1 || o.y >= ScreenHeight() - 1); });
			if (i != vecBullets.end())
				vecBullets.erase(i);
		}

		// Desenha disparos
		for (auto b : vecBullets)
			Draw(b.x, b.y);

		// Desenha nave
		DrawWireFrameModel(vecModelShip, player.x, player.y, player.angle);

		// Desenha pontuação
		DrawString(2, 2, L"SCORE: " + to_wstring(nScore));
		return true;
	}

	void DrawWireFrameModel(const vector<pair<float, float>>& vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, short col = FG_WHITE)
	{

		// Vetor, coordenadas
		vector<pair<float, float>> vecTransformedCoordinates;
		int verts = vecModelCoordinates.size();
		vecTransformedCoordinates.resize(verts);

		// Rotação
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
			vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
		}

		// Escala
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
		}

		// Conversão
		for (int i = 0; i < verts; i++)
		{
			vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
			vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
		}

		// Desenha polígono fechado
		for (int i = 0; i < verts + 1; i++)
		{
			int j = (i + 1);
			DrawLine(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second,
				vecTransformedCoordinates[j % verts].first, vecTransformedCoordinates[j % verts].second, PIXEL_SOLID, col);
		}
	}
};
